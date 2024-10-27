#include <database.h>
// Function to retrieve the chat list from the database for a user
cJSON *retrieve_chat_list(sqlite3 *db, int user_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT c.id, c.name, c.type FROM chats c "
                    "JOIN chat_members cm ON c.id = cm.chat_id "
                    "WHERE cm.user_id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, user_id);

  // Create JSON array for chats
  cJSON *chats = cJSON_CreateArray();
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int chat_id = sqlite3_column_int(stmt, 0);
    const char *chat_name = (const char *)sqlite3_column_text(stmt, 1);
    const char *chat_type = (const char *)sqlite3_column_text(stmt, 2);

    cJSON *chat = cJSON_CreateObject();
    cJSON_AddNumberToObject(chat, "chat_id", chat_id);
    cJSON_AddStringToObject(chat, "name", chat_name);
    cJSON_AddStringToObject(chat, "type", chat_type);
    cJSON_AddItemToArray(chats, chat);
  }
  sqlite3_finalize(stmt);

  return chats;
}