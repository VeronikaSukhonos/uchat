#include <uchat_server.h>

// Function to check if the user is logged in
int is_user_logged_in(sqlite3 *db, int user_id) {
  return logged_in(db,
                   user_id); // Assuming logged_in() checks user's login status
}

void delete_notifications_for_messages(sqlite3 *db, cJSON *chats_json,
                                       int user_id) {
  // Iterate over each chat in the chats JSON array
  cJSON *chat;
  cJSON_ArrayForEach(chat, chats_json) {
    // Get the chat_id for the current chat
    cJSON *chat_id_json = cJSON_GetObjectItem(chat, "chat_id");
    if (!chat_id_json) {
      continue; // Skip if chat_id is not found
    }
    int chat_id = chat_id_json->valueint;

    // Get the messages array for the current chat
    cJSON *messages_json = cJSON_GetObjectItem(chat, "messages");
    if (!cJSON_IsArray(messages_json)) {
      continue; // Skip if messages is not an array
    }

    // Iterate over each message in the current chat
    cJSON *message;
    cJSON_ArrayForEach(message, messages_json) {
      // Get the message_id for the current message
      cJSON *message_id_json = cJSON_GetObjectItem(message, "message_id");
      if (!message_id_json) {
        continue; // Skip if message_id is not found
      }
      int message_id = message_id_json->valueint;

      // SQL query to delete notifications for the given user and message_id
      const char *delete_sql =
          "DELETE FROM notifications WHERE user_id = ? AND message_id = ?;";

      sqlite3_stmt *delete_stmt;

      if (sqlite3_prepare_v2(db, delete_sql, -1, &delete_stmt, NULL) ==
          SQLITE_OK) {
        // Bind the user_id and message_id to the prepared statement
        sqlite3_bind_int(delete_stmt, 1, user_id);
        sqlite3_bind_int(delete_stmt, 2, message_id);

        // Execute the delete statement
        int result = sqlite3_step(delete_stmt);
        if (result != SQLITE_DONE) {
          fprintf(stderr,
                  "Failed to delete notification for message_id %d: %s\n",
                  message_id, sqlite3_errmsg(db));
        } else {
          printf("Notification deleted for user %d, message_id %d\n", user_id,
                 message_id);
        }

        sqlite3_finalize(delete_stmt);
      } else {
        fprintf(stderr,
                "Failed to prepare delete statement for message_id %d: %s\n",
                message_id, sqlite3_errmsg(db));
      }
    }
  }
}

// Function to handle retrieving the chat list for a use

int handle_get_chat_list(sqlite3 *db, Client *client) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  // Check if user is logged in
  if (!is_user_logged_in(db, user_id)) {
    fprintf(stderr, "User is not logged in.\n");
    return 1;
  }

  // Create JSON response
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CHAT_LIST");

  // Retrieve chat list from database
  cJSON *chats = retrieve_chat_list(db, user_id);
  if (chats == NULL) {
    cJSON_Delete(response);
    return 1;
  }
  cJSON_AddItemToObject(response, "chats", chats);

  // Send response to client
  char *response_str = cJSON_Print(response);
  printf("Sent: %s", response_str);
  send(client->socket, response_str, strlen(response_str), 0);
  delete_notifications_for_messages(db, chats, user_id);
  free(response_str);
  cJSON_Delete(response);

  return 0;
}