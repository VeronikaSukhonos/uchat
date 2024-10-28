#include <database.h>

// Function to retrieve online members of a specific chat
cJSON *get_online_chat_members(sqlite3 *db, int chat_id) {
  sqlite3_stmt *stmt;
  const char *sql =
      "SELECT u.username FROM chat_members cm "
      "JOIN users u ON cm.user_id = u.id "
      "WHERE cm.chat_id = ? AND u.last_seen = '1970-01-01 00:00:00';";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for online members: %s\n",
            sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, chat_id);
  cJSON *online_members = cJSON_CreateArray();

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *username = (const char *)sqlite3_column_text(stmt, 0);
    cJSON_AddItemToArray(online_members, cJSON_CreateString(username));
  }

  sqlite3_finalize(stmt);
  return online_members;
}

// Function to retrieve all members of a specific chat
cJSON *get_chat_members(sqlite3 *db, int chat_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT u.username FROM chat_members cm "
                    "JOIN users u ON cm.user_id = u.id "
                    "WHERE cm.chat_id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for chat members: %s\n",
            sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, chat_id);
  cJSON *chat_members = cJSON_CreateArray();

  // Retrieve each username and add it to the JSON array
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *username = (const char *)sqlite3_column_text(stmt, 0);
    cJSON_AddItemToArray(chat_members, cJSON_CreateString(username));
  }

  sqlite3_finalize(stmt);
  return chat_members;
}
