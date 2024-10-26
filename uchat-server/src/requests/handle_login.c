#include <uchat_server.h>

#define LOGIN_SUCCESS 0
#define LOGIN_FAILURE 1

int handle_login(sqlite3 *db, cJSON *json, char *session_token,
                 Client *client) {
  // Retrieve and check the 'username' field
  cJSON *username = cJSON_GetObjectItem(json, "username");
  if (!username || !cJSON_IsString(username)) {
    fprintf(stderr, "Invalid or missing username in JSON.\n");
    return LOGIN_FAILURE;
  }

  // Check if the username already exists
  int result = search_username(db, username->valuestring);
  if (result == 0) {
    return LOGIN_FAILURE; // Username exists
  } else if (result == -1) {
    return LOGIN_FAILURE; // Error occurred during search
  }

  // Retrieve and check 'email' and 'password' fields
  cJSON *password = cJSON_GetObjectItem(json, "password");
  if (!password || !cJSON_IsString(password)) {
    fprintf(stderr, "Invalid or missing password in JSON.\n");
    return LOGIN_FAILURE;
  }

  // Hash the password
  char hashed_password[SHA256_HASH_SIZE];
  hash_password(password->valuestring, hashed_password);
  // Attempt to register the user
  if (login_user(db, username->valuestring, client->serial_number,
                 client->ip_address, hashed_password, session_token,
                 sizeof(session_token)) == 0) {
    return LOGIN_SUCCESS; // Registration successful
  }

  return LOGIN_FAILURE; // Registration failed
}
