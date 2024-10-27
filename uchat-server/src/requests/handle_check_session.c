#include <uchat_server.h>

int handle_check_session(sqlite3 *db, cJSON *json, char *session_token,
                         Client *client) {
  if (!(client && json && session_token && db)) {
    fprintf(stderr, "Invalid arguments passed to handle_check_session.\n");
    return 1;
  }

  cJSON *token = cJSON_GetObjectItem(json, "session_token");
  strcpy(session_token, token->valuestring);

  // Extract username from JSON and get user_id
  cJSON *username_json = cJSON_GetObjectItem(json, "username");
  if (!cJSON_IsString(username_json) || username_json->valuestring == NULL) {
    fprintf(stderr, "Username not found in JSON.\n");
    return 1;
  }
  const char *username = username_json->valuestring;
  printf("Debug: Extracted username from JSON: %s\n", username);

  int user_id = get_user_id(db, username);
  if (user_id == -1) {
    fprintf(stderr, "User not found for username: %s\n", username);
    return 1;
  }

  char db_ip_address[50], db_serial_number[SHA256_HASH_SIZE], expires_at[30];
  int session_id;

  // Retrieve session details
  if (get_session_details(db, user_id, session_token, client->ip_address,
                          db_ip_address, db_serial_number, expires_at,
                          &session_id) != 0) {
    printf("Debug: No matching row found in database for session_token.\n");
    return 1; // Session is invalid
  }

  // Hash the client's serial number and compare with the database
  char hashed_serial[SHA256_HASH_SIZE];
  hash_password(client->serial_number, hashed_serial);

  if (strcmp(client->ip_address, db_ip_address) == 0 &&
      strcmp(hashed_serial, db_serial_number) == 0) {
    printf("Debug: IP address and serial number match.\n");

    // Check if the session is expired
    char current_time[30];
    if (get_current_time(db, current_time) != 0) {
      fprintf(stderr, "Failed to retrieve current time.\n");
      return 1;
    }

    if (strcmp(current_time, expires_at) < 0) {
      printf("Debug: Session is still valid.\n");
      update_last_seen(db, user_id, 0);
      return 0; // Session is valid
    } else {
      printf("Debug: Session has expired. Deleting session.\n");
      delete_expired_session(db, session_id);
    }
  } else {
    printf("Debug: IP address or serial number does not match.\n");
  }

  return 1; // Session is invalid
}