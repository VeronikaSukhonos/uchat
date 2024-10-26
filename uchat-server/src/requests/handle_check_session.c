#include <uchat_server.h>

int handle_check_session(sqlite3 *db, cJSON *json, char *session_token,
                         Client *client) {
  if (!(client && json && session_token && db)) {
    fprintf(stderr, "Invalid arguments passed to handle_check_session.\n");
    return 1; // Invalid arguments
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
    return 1; // User not found
  }
  printf("Debug: Retrieved user_id for username '%s': %d\n", username, user_id);

  // Prepare SQL query to check IP address, serial number, and session
  // expiration
  sqlite3_stmt *stmt;
  const char *sql = "SELECT u.ip_address, u.serial_number, s.expires_at "
                    "FROM user_ips AS u "
                    "JOIN sessions AS s ON s.user_id = u.user_id "
                    "WHERE u.user_id = ? AND s.token = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, session_token, -1, SQLITE_STATIC);
  printf("Debug: Bound user_id (%d) and session_token (%s) to SQL statement.\n",
         user_id, session_token);

  int result = sqlite3_step(stmt);
  if (result == SQLITE_ROW) {
    const char *db_ip_address = (const char *)sqlite3_column_text(stmt, 0);
    const char *db_serial_number = (const char *)sqlite3_column_text(stmt, 1);
    const char *expires_at = (const char *)sqlite3_column_text(stmt, 2);

    printf("Debug: Retrieved values from DB - IP: %s, Serial: %s, Expires At: "
           "%s\n",
           db_ip_address, db_serial_number, expires_at);
    printf("Debug: Client values - IP: %s, Serial: %s\n", client->ip_address,
           client->serial_number);

    // Compare IP address and serial number
    if (strcmp(client->ip_address, db_ip_address) == 0 &&
        strcmp(client->serial_number, db_serial_number) == 0) {
      printf("Debug: IP address and serial number match.\n");

      // Check if session is expired
      const char *current_time_query = "SELECT datetime('now')";
      sqlite3_stmt *time_stmt;
      if (sqlite3_prepare_v2(db, current_time_query, -1, &time_stmt, NULL) ==
          SQLITE_OK) {
        if (sqlite3_step(time_stmt) == SQLITE_ROW) {
          const char *current_time =
              (const char *)sqlite3_column_text(time_stmt, 0);
          printf("Debug: Current time: %s, Session expires at: %s\n",
                 current_time, expires_at);
          if (strcmp(current_time, expires_at) < 0) {
            printf("Debug: Session is still valid.\n");
            sqlite3_finalize(stmt);
            sqlite3_finalize(time_stmt);
            update_last_seen(db, user_id, 0);
            return 0; // Session is valid
          } else {
            printf("Debug: Session has expired.\n");
          }
        }
        sqlite3_finalize(time_stmt);
      } else {
        fprintf(stderr, "Failed to prepare time query statement: %s\n",
                sqlite3_errmsg(db));
      }
    } else {
      printf("Debug: IP address or serial number does not match.\n");
    }
  } else {
    printf("Debug: No matching row found in database for session_token.\n");
  }

  sqlite3_finalize(stmt);
  return 1; // Session is invalid
}
