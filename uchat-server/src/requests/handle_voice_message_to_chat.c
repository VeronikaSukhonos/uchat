#include <uchat_server.h>

int handle_voice_message_to_chat(sqlite3 *db, Client *client, cJSON *json,
                                 Client clients[], int max_clients) {
  // Extract chat_id and Base64 encoded file from JSON
  cJSON *chat_id_json = cJSON_GetObjectItem(json, "chat_id");
  cJSON *file_json = cJSON_GetObjectItem(json, "file");

  if (!cJSON_IsNumber(chat_id_json) || !cJSON_IsString(file_json)) {
    fprintf(stderr, "Invalid JSON format for sending voice message.\n");
    return 1;
  }

  int chat_id = chat_id_json->valueint;
  const char *encoded_file = file_json->valuestring;
  int sender_id = get_user_id(db, client->username);

  if (sender_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  // Decode the Base64 file
  size_t decoded_size;
  unsigned char *decoded_file =
      base64_decode(encoded_file, strlen(encoded_file), &decoded_size);
  if (!decoded_file) {
    fprintf(stderr, "Failed to decode the Base64 file.\n");
    return 1;
  }

  // Store the voice message in the database and retrieve the message_id
  int message_id =
      store_voice_message(db, chat_id, sender_id, decoded_file, decoded_size);
  free(decoded_file); // Clean up decoded file memory

  if (message_id == -1) {
    fprintf(stderr, "Failed to store voice message for chat ID %d\n", chat_id);
    return 1;
  }

  // Construct the file name
  char file_name[128];
  snprintf(file_name, sizeof(file_name), "chat_%d_%d_vmsg.wav", chat_id,
           message_id);

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

  // Add the file name to the message details
  cJSON_AddStringToObject(message_details, "file_name", file_name);

  // Prepare JSON response for "MESSAGE_FROM_CHAT"
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "file_name", file_name);
  cJSON_AddStringToObject(response, "action", "VOICE_FROM_CHAT");
  cJSON_AddNumberToObject(response, "chat_id", chat_id);
  cJSON_AddItemToObject(response, "message", message_details);

  char *response_str = cJSON_Print(response);

  // Send the voice message to each online client
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

    // Send to online clients only
    for (int j = 0; j < max_clients; j++) {
      if (clients[j].socket > 0 && strcmp(clients[j].username, username) == 0 &&
          strcmp(username, client->username) != 0) {
        if (send(clients[j].socket, response_str, strlen(response_str), 0) ==
            -1) {
          perror("Failed to send voice message to client");
        } else {
          printf("Voice message sent to %s\n", username);
        }
      }
    }
  }

  // Notify the sender with a "SEND_VOICE_MESSAGE_TO_SERVER_STATUS" action
  cJSON *sender_response = cJSON_CreateObject();
  cJSON_AddStringToObject(sender_response, "action",
                          "SEND_VOICE_MESSAGE_TO_SERVER_STATUS");
  cJSON_AddStringToObject(sender_response, "status", "SUCCESS");
  cJSON_AddNumberToObject(sender_response, "chat_id", chat_id);
  cJSON_AddStringToObject(sender_response, "file_name", file_name);
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