#include <cJSON.h>
#include <database.h>
#include <sqlite3.h>
#include <stdio.h>

cJSON *get_message_details(sqlite3 *db, int message_id) {
  const char *sql =
      "SELECT m.id, m.chat_id, sender_id, "
      "(SELECT username FROM users WHERE users.id = sender_id) AS username, "
      "m.content, m.type, m.created_at, m.is_read, m.voice_message, m.status "
      "FROM messages m WHERE m.id = ?;";
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
    cJSON_AddStringToObject(message, "type",
                            (const char *)sqlite3_column_text(stmt, 5));
    cJSON_AddStringToObject(message, "timestamp",
                            (const char *)sqlite3_column_text(stmt, 6));
    cJSON_AddBoolToObject(message, "read", sqlite3_column_int(stmt, 7));
    cJSON_AddStringToObject(message, "status",
                            (const char *)sqlite3_column_text(stmt, 9));

    const char *message_type = (const char *)sqlite3_column_text(stmt, 5);

    // Handle "voice" type
    if (strcmp(message_type, "voice") == 0) {
      const void *voice_message_blob = sqlite3_column_blob(stmt, 8);
      int blob_size = sqlite3_column_bytes(stmt, 8);

      if (voice_message_blob && blob_size > 0) {
        // Encode the binary data into Base64 for sending over JSON
        char *encoded_voice_message =
            base64_encode(voice_message_blob, blob_size);
        if (encoded_voice_message) {
          cJSON_AddStringToObject(message, "voice_message",
                                  encoded_voice_message);
          free(encoded_voice_message); // Free the encoded string after use
        }
      } else {
        cJSON_AddStringToObject(message, "voice_message", "");
      }
    }

    // Handle "file" type
    if (strcmp(message_type, "file") == 0) {
      const char *sql_file =
          "SELECT file_path, file_type, size, uploaded_at, file "
          "FROM files WHERE message_id = ?;";
      sqlite3_stmt *file_stmt;

      if (sqlite3_prepare_v2(db, sql_file, -1, &file_stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(file_stmt, 1, message_id);

        if (sqlite3_step(file_stmt) == SQLITE_ROW) {
          // Extract file details
          const char *file_path =
              (const char *)sqlite3_column_text(file_stmt, 0);
          const char *file_type =
              (const char *)sqlite3_column_text(file_stmt, 1);
          int file_size = sqlite3_column_int(file_stmt, 2);
          const char *uploaded_at =
              (const char *)sqlite3_column_text(file_stmt, 3);
          const void *file_blob = sqlite3_column_blob(file_stmt, 4);
          int blob_size = sqlite3_column_bytes(file_stmt, 4);

          cJSON_AddStringToObject(message, "file_path",
                                  file_path ? file_path : "");
          cJSON_AddStringToObject(message, "file_type",
                                  file_type ? file_type : "");
          // cJSON_AddNumberToObject(message, "file_size", file_size);
          // cJSON_AddStringToObject(message, "uploaded_at",
          //                         uploaded_at ? uploaded_at : "");

          if (file_blob && blob_size > 0) {
            // Encode the file data into Base64 for sending over JSON
            char *encoded_file = base64_encode(file_blob, blob_size);
            if (encoded_file) {
              cJSON_AddStringToObject(message, "file_data", encoded_file);
              free(encoded_file); // Free the encoded string after use
            }
          } else {
            cJSON_AddStringToObject(message, "file_data", "");
          }
        }

        sqlite3_finalize(file_stmt);
      } else {
        fprintf(stderr, "Failed to prepare statement for file details: %s\n",
                sqlite3_errmsg(db));
      }
    }

    // Finalize the statement and return the JSON object
    sqlite3_finalize(stmt);
    return message;
  }

  // No results found
  fprintf(stderr, "No message found with ID %d\n", message_id);
  sqlite3_finalize(stmt);
  return NULL;
}