#include "uchat.h"

// Function to read and handle server responses
int handle_response(int sock, int *logged_in, AppData *app_data) {
  char buffer[BUFFER_SIZE];
  int valread = read(sock, buffer, sizeof(buffer) - 1);

  if (valread <= 0) {
    printf("Server disconnected or no data received.\n");
    *logged_in = 0;
    return -1;
  }

  buffer[valread] = '\0';
  cJSON *response = cJSON_Parse(buffer);
  if (response == NULL) {
    printf("Failed to parse server response.\n");
    return -1;
  }

  cJSON *action = cJSON_GetObjectItem(response, "action");
  if (action && cJSON_IsString(action)) {
    if (strcmp(action->valuestring, "LOGIN") == 0) {
      cJSON *status = cJSON_GetObjectItem(response, "status");
      if (strcmp(status->valuestring, "SUCCESS") == 0) {
        char session_token[33];
        cJSON *username = cJSON_GetObjectItem(response, "username");
        g_print("Login successful.\n");

        // Get the session token from the response
        cJSON *token = cJSON_GetObjectItem(response, "session_token");
        if (token && cJSON_IsString(token)) {
          strcpy(session_token, token->valuestring);
          save_session(username->valuestring, session_token);
          *logged_in = 1;
          return 1;
        } else {
          g_print("Error: No session token received.\n");
          return -1;
        }
      } else {
        g_print("Error: Login Error.\n");
        gtk_label_set_text(GTK_LABEL(app_data->login_data->message),
                           "Wrong password or login!");
      }

    } else if (strcmp(action->valuestring, "REGISTER") == 0) {
      cJSON *status = cJSON_GetObjectItem(response, "status");
      if (strcmp(status->valuestring, "SUCCESS") == 0) {
        g_print("Registration successful\n");
        gtk_label_set_text(GTK_LABEL(app_data->registration_data->message),
                           "Please login!");
      } else {
        g_print("Error: Registration error.\n");
        gtk_label_set_text(GTK_LABEL(app_data->registration_data->message),
                           "Choose another username!");
      }

    } else if (strcmp(action->valuestring, "CHAT_LIST") == 0) {
      cJSON *chats = cJSON_GetObjectItem(response, "chats");
      if (cJSON_IsArray(chats)) {
        printf("Your Chats:\n");
        int chat_count = cJSON_GetArraySize(chats);
        for (int i = 0; i < chat_count; i++) {
          cJSON *chat = cJSON_GetArrayItem(chats, i);
          cJSON *chat_id = cJSON_GetObjectItem(chat, "chat_id");
          cJSON *chat_name = cJSON_GetObjectItem(chat, "name");
          cJSON *chat_type = cJSON_GetObjectItem(chat, "type");

          if (chat_id && chat_name && chat_type) {
            printf("Chat ID: %d, Name: %s, Type: %s\n", chat_id->valueint,
                   chat_name->valuestring, chat_type->valuestring);
          }
        }

        // Ask user if they want to message or close a chat
        printf("Enter Chat ID to message, or 0 to go back: ");
        int selected_chat_id;
        scanf("%d", &selected_chat_id);
        getchar(); // Consume newline

        if (selected_chat_id > 0) {
          printf("Enter message: ");
          char message[256];
          fgets(message, sizeof(message), stdin);
          message[strcspn(message, "\n")] = 0;

          cJSON *msg_json = cJSON_CreateObject();
          cJSON_AddStringToObject(msg_json, "action", "SEND_MESSAGE_TO_CHAT");
          cJSON_AddNumberToObject(msg_json, "chat_id", selected_chat_id);
          cJSON_AddStringToObject(msg_json, "message", message);
          char *msg_str = cJSON_Print(msg_json);
          cJSON_Delete(msg_json);

          send(sock, msg_str, strlen(msg_str), 0);
          printf("Sent message to Chat ID %d: %s\n", selected_chat_id, msg_str);
          free(msg_str);
        }
      } else {
        printf("Unknown action: %s\n", action->valuestring);
      }

    } else if (strcmp(action->valuestring, "MESSAGE_FROM_CHAT") == 0) {
      // Handle receiving a message from a chat
      cJSON *chat_id = cJSON_GetObjectItem(response, "chat_id");
      cJSON *sender = cJSON_GetObjectItem(response, "sender");
      cJSON *message = cJSON_GetObjectItem(response, "message");

      if (chat_id && sender && message) {
        printf("New message in Chat ID %d from %s: %s\n", chat_id->valueint,
               sender->valuestring, message->valuestring);
      } else {
        printf("Error: Missing fields in MESSAGE_FROM_CHAT response.\n");
      }
    }

  } else {
    printf("Invalid or missing 'action' field in server response.\n");
  }

  // Clean up JSON object
  cJSON_Delete(response);
  return 0;
}
