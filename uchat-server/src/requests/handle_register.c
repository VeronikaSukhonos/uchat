#include <uchat_server.h>

#define REGISTRATION_SUCCESS 0
#define REGISTRATION_FAILURE 1

int handle_register(sqlite3 *db, cJSON *json) {
  // Retrieve and check the 'username' field
  cJSON *username = cJSON_GetObjectItem(json, "username");
  if (!username || !cJSON_IsString(username)) {
    fprintf(stderr, "Invalid or missing username in JSON.\n");
    return REGISTRATION_FAILURE;
  }

  // Check if the username already exists
  int result = search_username(db, username->valuestring);
  if (result == 1) {
    return REGISTRATION_FAILURE; // Username exists
  } else if (result == -1) {
    return REGISTRATION_FAILURE; // Error occurred during search
  }

  // Retrieve and check 'email' and 'password' fields
  cJSON *password = cJSON_GetObjectItem(json, "password");
  if (!password || !cJSON_IsString(password)) {
    fprintf(stderr, "Invalid or missing email/password in JSON.\n");
    return REGISTRATION_FAILURE;
  }

  // Hash the password
  char hashed_password[SHA256_HASH_SIZE];
  hash_password(password->valuestring, hashed_password);

  // Attempt to register the user
  if (register_user(db, username->valuestring, hashed_password) == 0) {
    return REGISTRATION_SUCCESS; // Registration successful
  }

  return REGISTRATION_FAILURE; // Registration failed
}
