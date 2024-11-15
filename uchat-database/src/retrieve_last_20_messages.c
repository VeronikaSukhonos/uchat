#include <database.h>

// Function to retrieve the last 20 messages for a given chat
cJSON *retrieve_last_20_messages(sqlite3 *db, int chat_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT m.id, m.sender_id, u.username, m.content, "
                    "m.type, m.created_at, m.is_read "
                    "FROM messages m "
                    "JOIN users u ON m.sender_id = u.id "
                    "WHERE m.chat_id = ? "
                    "ORDER BY m.created_at DESC "
                    "LIMIT 20;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, chat_id);

  // Create JSON array for messages
  cJSON *messages = cJSON_CreateArray();
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int message_id = sqlite3_column_int(stmt, 0);
    int sender_id = sqlite3_column_int(stmt, 1);
    const char *username = (const char *)sqlite3_column_text(stmt, 2);
    const char *content = (const char *)sqlite3_column_text(stmt, 3);
    const char *content_type = (const char *)sqlite3_column_text(stmt, 4);
    const char *timestamp = (const char *)sqlite3_column_text(stmt, 5);
    int is_read = sqlite3_column_int(stmt, 6);

    cJSON *message = cJSON_CreateObject();
    cJSON_AddNumberToObject(message, "message_id", message_id);
    cJSON_AddNumberToObject(message, "sender_id", sender_id);
    cJSON_AddStringToObject(message, "username", username);
    cJSON_AddStringToObject(message, "content", content);
    cJSON_AddStringToObject(message, "content_type", content_type);
    cJSON_AddStringToObject(message, "timestamp", timestamp);
    cJSON_AddBoolToObject(message, "read", is_read);

    cJSON_AddItemToArray(messages, message);
  }
  sqlite3_finalize(stmt);

  return messages;
}