#include <uchat_server.h>

#define PROFILE_UPDATE_SUCCESS 0
#define PROFILE_UPDATE_FAILURE 1

int handle_update_profile(sqlite3 *db, Client *client, cJSON *json) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "Invalid session token.\n");
    return PROFILE_UPDATE_FAILURE;
  }

  // Extract new data from the JSON
  cJSON *new_username_json = cJSON_GetObjectItem(json, "username");
  cJSON *full_name_json = cJSON_GetObjectItem(json, "full_name");
  cJSON *group_json = cJSON_GetObjectItem(json, "group");
  cJSON *role_json = cJSON_GetObjectItem(json, "role");

  if (!cJSON_IsString(new_username_json) || !cJSON_IsString(full_name_json) ||
      !cJSON_IsString(group_json) || !cJSON_IsString(role_json)) {
    fprintf(stderr, "Invalid data in JSON.\n");
    return PROFILE_UPDATE_FAILURE;
  }

  const char *new_username = new_username_json->valuestring;
  const char *full_name = full_name_json->valuestring;
  const char *group = group_json->valuestring;
  const char *role = role_json->valuestring;

  // Check if the new username is already taken if it's different from the
  // current one
  int username_changed = 0;
  if (strcmp(client->username, new_username) != 0) {
    int id = get_user_id(db, new_username);

    if (id > 0) {
      // Username is taken, return failure response
      cJSON *response = cJSON_CreateObject();
      cJSON_AddStringToObject(response, "action", "UPDATE_PROFILE_DATA");
      cJSON_AddStringToObject(response, "status", "FAILURE");
      cJSON_AddStringToObject(response, "error", "Username already taken");
      send_json_responce_to_client(client, response);
      return PROFILE_UPDATE_FAILURE;
    }
    username_changed = 1; // Mark that username is being changed
  }

  // Prepare and execute the update query
  char *update_query = "UPDATE users SET username = ?, full_name = ?, `group` "
                       "= ?, role = ? WHERE id = ?";
  sqlite3_stmt *update_stmt;
  if (sqlite3_prepare_v2(db, update_query, -1, &update_stmt, NULL) !=
      SQLITE_OK) {
    fprintf(stderr, "Failed to prepare update statement: %s\n",
            sqlite3_errmsg(db));
    return PROFILE_UPDATE_FAILURE;
  }

  sqlite3_bind_text(update_stmt, 1, new_username, -1, SQLITE_STATIC);
  sqlite3_bind_text(update_stmt, 2, full_name, -1, SQLITE_STATIC);
  sqlite3_bind_text(update_stmt, 3, group, -1, SQLITE_STATIC);
  sqlite3_bind_text(update_stmt, 4, role, -1, SQLITE_STATIC);
  sqlite3_bind_int(update_stmt, 5, user_id);

  int update_result = sqlite3_step(update_stmt);
  sqlite3_finalize(update_stmt); // Ensure the statement is finalized

  if (update_result != SQLITE_DONE) {
    fprintf(stderr, "Failed to update user profile: %s\n", sqlite3_errmsg(db));
    return PROFILE_UPDATE_FAILURE;
  }

  // change username in chats names
  char *chats_update_query = "UPDATE chats SET name = REPLACE(name, ?, ?)";
  sqlite3_stmt *chats_update_stmt;
  if (sqlite3_prepare_v2(db, chats_update_query, -1, &chats_update_stmt, NULL) !=
      SQLITE_OK) {
    fprintf(stderr, "Failed to prepare chats update statement: %s\n",
            sqlite3_errmsg(db));
    return PROFILE_UPDATE_FAILURE;
  }

  sqlite3_bind_text(chats_update_stmt, 1, client->username, -1, SQLITE_STATIC);
  sqlite3_bind_text(chats_update_stmt, 2, new_username, -1, SQLITE_STATIC);

  update_result = sqlite3_step(chats_update_stmt);
  sqlite3_finalize(chats_update_stmt);

  if (update_result != SQLITE_DONE) {
    fprintf(stderr, "Failed to update chats name: %s\n", sqlite3_errmsg(db));
    return PROFILE_UPDATE_FAILURE;
  }

  // Send success response
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "UPDATE_PROFILE_DATA");
  cJSON_AddStringToObject(response, "username_change",
                          username_changed ? "TRUE" : "FALSE");
  if (username_changed) {
    strcpy(client->username, new_username);
  }
  cJSON_AddStringToObject(response, "status", "SUCCESS");
  send_json_responce_to_client(client, response);

  printf("Profile updated successfully for user ID %d.\n", user_id);
  return PROFILE_UPDATE_SUCCESS;
}
