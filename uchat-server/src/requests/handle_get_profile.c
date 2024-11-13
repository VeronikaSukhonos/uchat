#include <uchat_server.h>

#define PROFILE_FETCH_SUCCESS 0
#define PROFILE_FETCH_FAILURE 1

int handle_get_profile(sqlite3 *db, Client *client) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "Invalid session token.\n");
    return PROFILE_FETCH_FAILURE;
  }

  // Updated query to fetch full_name, group, and role along with username
  char *query =
      "SELECT username, full_name, `group`, role FROM users WHERE id = ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return PROFILE_FETCH_FAILURE;
  }

  sqlite3_bind_int(stmt, 1, user_id);

  int result = sqlite3_step(stmt);
  if (result != SQLITE_ROW) {
    fprintf(stderr, "Failed to retrieve user data: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return PROFILE_FETCH_FAILURE;
  }

  // Retrieve values from the query result
  const char *username = (const char *)sqlite3_column_text(stmt, 0);
  const char *full_name = (const char *)sqlite3_column_text(stmt, 1);
  const char *group = (const char *)sqlite3_column_text(stmt, 2);
  const char *role = (const char *)sqlite3_column_text(stmt, 3);

  // Create a JSON response with the retrieved values
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "GET_PROFILE_DATA");
  cJSON_AddStringToObject(response, "status", "SUCCESS");
  cJSON_AddStringToObject(response, "username", username ? username : "");
  cJSON_AddStringToObject(response, "full_name", full_name ? full_name : "");
  cJSON_AddStringToObject(response, "group", group ? group : "");
  cJSON_AddStringToObject(response, "role", role ? role : "");

  // Send JSON response to the client
  send_json_responce_to_client(client, response);

  // Cleanup
  sqlite3_finalize(stmt);

  return PROFILE_FETCH_SUCCESS;
}