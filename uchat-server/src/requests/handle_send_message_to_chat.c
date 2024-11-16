#include <uchat_server.h>

int handle_send_message_to_chat(sqlite3 *db, Client *client, cJSON *json,
                                Client clients[], int max_clients) {
  cJSON *chat_id_json = cJSON_GetObjectItem(json, "chat_id");
  cJSON *content_json = cJSON_GetObjectItem(json, "message");

  if (!cJSON_IsNumber(chat_id_json) || !cJSON_IsString(content_json)) {
    fprintf(stderr, "Invalid JSON format for sending message.\n");
    return 1;
  }

  int chat_id = chat_id_json->valueint;
  const char *content = content_json->valuestring;
  int sender_id = get_user_id(db, client->username);

  if (sender_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  // Store the message in the database
  int message_id = store_message(db, chat_id, sender_id, content);
  if (message_id == -1) {
    fprintf(stderr, "Failed to store message for chat ID %d\n", chat_id);
    return 1;
  }

  // Retrieve online members of the chat
  cJSON *members = get_chat_members(db, chat_id);
  if (!members) {
    fprintf(stderr, "Failed to retrieve members for chat ID %d\n", chat_id);
    return 1;
  }

  // Retrieve the stored message details
  cJSON *message_details = get_message_details(db, message_id);
  if (!message_details) {
    fprintf(stderr, "Failed to retrieve message details for message ID %d\n",
            message_id);
    cJSON_Delete(members);
    return 1;
  }

  // Prepare JSON response for "MESSAGE_FROM_CHAT"
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "MESSAGE_FROM_CHAT");
  cJSON_AddNumberToObject(response, "chat_id", chat_id);
  cJSON_AddItemToObject(response, "message", message_details);

  char *response_str = cJSON_Print(response);

  // Send the message to each online client and create notifications
  for (int i = 0; i < cJSON_GetArraySize(members); i++) {
    cJSON *username_json = cJSON_GetArrayItem(members, i);
    const char *username = cJSON_GetStringValue(username_json);
    int recipient_id = get_user_id(db, username);

    if (recipient_id == -1) {
      fprintf(stderr, "User not found: %s\n", username);
      continue;
    }

    // Store the notification
    if (strcmp(username, client->username) != 0)
      if (store_notification(db, recipient_id, message_id) == -1) {
        fprintf(stderr, "Failed to store notification for user ID %d\n",
                recipient_id);
      }

    // Send to online clients only and update is_delivered status
    for (int j = 0; j < max_clients; j++) {
      if (clients[j].socket > 0 &&
          strcmp(clients[j].username, client->username) != 0) {
        if (send(clients[j].socket, response_str, strlen(response_str), 0) ==
            -1) {
          perror("Failed to send message to client");
        } else {
          // Update the notification as delivered
          const char *update_sql = "UPDATE notifications SET is_delivered = 1 "
                                   "WHERE user_id = ? AND message_id = ?;";
          sqlite3_stmt *update_stmt;
          if (sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, NULL) ==
              SQLITE_OK) {
            sqlite3_bind_int(update_stmt, 1, recipient_id);
            sqlite3_bind_int(update_stmt, 2, message_id);
            sqlite3_step(update_stmt);
            sqlite3_finalize(update_stmt);
          }
        }
      }
    }
  }

  // Notify the sender with a "SEND_MESSAGE_TO_SERVER_STATUS" action
  cJSON *sender_response = cJSON_CreateObject();
  cJSON_AddStringToObject(sender_response, "action",
                          "SEND_MESSAGE_TO_SERVER_STATUS");
  cJSON_AddStringToObject(sender_response, "status", "SUCCESS");
  cJSON_AddNumberToObject(sender_response, "chat_id", chat_id);
  cJSON_AddItemToObject(sender_response, "message",
                        cJSON_Duplicate(message_details, 1));

  char *sender_response_str = cJSON_Print(sender_response);
  if (send(client->socket, sender_response_str, strlen(sender_response_str),
           0) == -1) {
    perror("Failed to send status to sender");
  }

  free(response_str);
  free(sender_response_str);
  cJSON_Delete(sender_response);
  cJSON_Delete(response);
  cJSON_Delete(members);

  return 0;
}
