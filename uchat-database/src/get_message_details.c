#include <cJSON.h>
#include <database.h>
#include <sqlite3.h>
#include <stdio.h>

cJSON *get_message_details(sqlite3 *db, int message_id) {
  const char *sql =
      "SELECT id, chat_id, sender_id, "
      "(SELECT username FROM users WHERE id = sender_id) AS username, "
      "content, type, created_at, is_read "
      "FROM messages WHERE id = ?;";
  sqlite3_stmt *stmt;

  // Prepare the SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return NULL;
  }

  // Bind the message_id parameter
  sqlite3_bind_int(stmt, 1, message_id);

  // Execute the query and fetch the result
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    // Create a JSON object to hold the message details
    cJSON *message = cJSON_CreateObject();

    // Extract message details from the result row
    cJSON_AddNumberToObject(message, "message_id", sqlite3_column_int(stmt, 0));
    cJSON_AddNumberToObject(message, "sender_id", sqlite3_column_int(stmt, 2));
    cJSON_AddStringToObject(message, "username",
                            (const char *)sqlite3_column_text(stmt, 3));
    cJSON_AddStringToObject(message, "content",
                            (const char *)sqlite3_column_text(stmt, 4));
    cJSON_AddStringToObject(message, "content_type",
                            (const char *)sqlite3_column_text(stmt, 5));
    cJSON_AddStringToObject(message, "timestamp",
                            (const char *)sqlite3_column_text(stmt, 6));
    cJSON_AddBoolToObject(message, "read", sqlite3_column_int(stmt, 7));

    // Finalize the statement and return the JSON object
    sqlite3_finalize(stmt);
    return message;
  }

  // No results found
  fprintf(stderr, "No message found with ID %d\n", message_id);
  sqlite3_finalize(stmt);
  return NULL;
}