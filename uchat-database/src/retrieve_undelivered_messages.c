#include <database.h>

cJSON *retrieve_undelivered_messages(sqlite3 *db, int user_id, int chat_id) {
  const char *sql_messages =
      "SELECT m.id, m.sender_id, "
      "(SELECT username FROM users WHERE id = m.sender_id) AS username, "
      "m.content, m.type, m.created_at, m.is_read, "
      "m.voice_message, m.status "
      "FROM messages m "
      "JOIN notifications n ON m.id = n.message_id "
      "WHERE m.chat_id = ? AND n.user_id = ? "
      "ORDER BY m.created_at DESC";

  sqlite3_stmt *stmt;

  // Prepare the statement
  if (sqlite3_prepare_v2(db, sql_messages, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(
        stderr,
        "Failed to prepare statement for retrieving undelivered messages: %s\n",
        sqlite3_errmsg(db));
    return NULL;
  }

  printf("Binding chat_id: %d, user_id: %d\n", chat_id, user_id);

  // Bind parameters
  sqlite3_bind_int(stmt, 1, chat_id); // Bind the chat_id
  sqlite3_bind_int(stmt, 2, user_id); // Bind the user_id

  // Create a JSON array for the undelivered messages
  cJSON *messages = cJSON_CreateArray();

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    printf("Retrieved message_id=%d\n", sqlite3_column_int(stmt, 0));
    cJSON *message = cJSON_CreateObject();

    int message_id = sqlite3_column_int(stmt, 0);
    int sender_id = sqlite3_column_int(stmt, 1);
    const char *username = (const char *)sqlite3_column_text(stmt, 2);
    const char *content = (const char *)sqlite3_column_text(stmt, 3);
    const char *type = (const char *)sqlite3_column_text(stmt, 4);
    const char *created_at = (const char *)sqlite3_column_text(stmt, 5);
    int is_read = sqlite3_column_int(stmt, 6);
    const char *status = (const char *)sqlite3_column_text(stmt, 8);

    // Add basic message fields
    cJSON_AddNumberToObject(message, "message_id", message_id);
    cJSON_AddNumberToObject(message, "sender_id", sender_id);
    cJSON_AddStringToObject(message, "username", username ? username : "");
    cJSON_AddStringToObject(message, "content", content ? content : "");
    cJSON_AddStringToObject(message, "type", type);
    cJSON_AddStringToObject(message, "timestamp", created_at);
    cJSON_AddBoolToObject(message, "read", is_read);
    cJSON_AddStringToObject(message, "status", status);

    if (strcmp(type, "voice") == 0) {
      // Handle voice messages
      const void *voice_message_blob = sqlite3_column_blob(stmt, 7);
      int voice_message_size = sqlite3_column_bytes(stmt, 7);
      if (voice_message_blob && voice_message_size > 0) {
        char file_path[128] = "";
        snprintf(file_path, sizeof(file_path), "chat_%d_%d_vmsg.wav", chat_id,
                 message_id);
        cJSON_AddStringToObject(message, "file_name", file_path);

        char *encoded_voice = base64_encode(
            (const unsigned char *)voice_message_blob, voice_message_size);
        if (encoded_voice) {
          cJSON_AddStringToObject(message, "voice_message", encoded_voice);
          free(encoded_voice);
        } else {
          cJSON_AddStringToObject(message, "voice_message", "");
        }
      } else {
        cJSON_AddStringToObject(message, "voice_message", "");
        cJSON_AddStringToObject(message, "file_name", "");
      }
    } else if (strcmp(type, "file") == 0) {
      // Handle file messages
      sqlite3_stmt *file_stmt;
      const char *file_sql =
          "SELECT file_path, file_type, file FROM files WHERE message_id = ?;";
      if (sqlite3_prepare_v2(db, file_sql, -1, &file_stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement for retrieving file: %s\n",
                sqlite3_errmsg(db));
        continue;
      }

      sqlite3_bind_int(file_stmt, 1, message_id);
      if (sqlite3_step(file_stmt) == SQLITE_ROW) {
        const char *file_path = (const char *)sqlite3_column_text(file_stmt, 0);
        const char *file_type = (const char *)sqlite3_column_text(file_stmt, 1);
        const void *file_blob = sqlite3_column_blob(file_stmt, 2);
        int file_size = sqlite3_column_bytes(file_stmt, 2);

        cJSON_AddStringToObject(message, "file_path",
                                file_path ? file_path : "");
        cJSON_AddStringToObject(message, "file_type",
                                file_type ? file_type : "");

        if (file_blob && file_size > 0) {
          // Base64-encode the file for JSON
          char *encoded_file =
              base64_encode((const unsigned char *)file_blob, file_size);
          if (encoded_file) {
            cJSON_AddStringToObject(message, "file_data", encoded_file);
            free(encoded_file);
          } else {
            cJSON_AddStringToObject(message, "file_data", "");
          }
        }
      } else {
        cJSON_AddStringToObject(message, "file_path", "");
        cJSON_AddStringToObject(message, "file_type", "");
        cJSON_AddStringToObject(message, "file_data", "");
      }
      sqlite3_finalize(file_stmt);
    }

    cJSON_AddItemToArray(messages, message);

    // Step 2: Delete the notification for the current message
    const char *delete_sql =
        "DELETE FROM notifications WHERE user_id = ? AND message_id = ?";
    sqlite3_stmt *delete_stmt;
    if (sqlite3_prepare_v2(db, delete_sql, -1, &delete_stmt, NULL) ==
        SQLITE_OK) {
      sqlite3_bind_int(delete_stmt, 1, user_id);    // Bind the user_id
      sqlite3_bind_int(delete_stmt, 2, message_id); // Bind the message_id
      sqlite3_step(delete_stmt);                    // Execute the deletion
      sqlite3_finalize(delete_stmt);
    } else {
      fprintf(stderr,
              "Failed to prepare statement for deleting notification: %s\n",
              sqlite3_errmsg(db));
    }
  }

  sqlite3_finalize(stmt);
  return messages;
}