#include "uchat.h"

// Function to read and handle server responses
void handle_response(int sock, int *logged_in) {
  char buffer[BUFFER_SIZE];
  int valread = read(sock, buffer, sizeof(buffer) - 1);

  if (valread <= 0) {
    printf("Server disconnected or no data received.\n");
    *logged_in = 0;
    return;
  }

  buffer[valread] = '\0';
  cJSON *response = cJSON_Parse(buffer);
  if (response == NULL) {
    printf("Failed to parse server response.\n");
    return;
  }

  cJSON *action = cJSON_GetObjectItem(response, "action");
  if (action && cJSON_IsString(action)) {
    if (strcmp(action->valuestring, "LOGIN") == 0) {
      cJSON *status = cJSON_GetObjectItem(response, "status");
      if (strcmp(status->valuestring, "SUCCESS") == 0) {
        char session_token[33];
        cJSON *username = cJSON_GetObjectItem(response, "username");
        printf("Login successful.\n");

        // Get the session token from the response
        cJSON *token = cJSON_GetObjectItem(response, "session_token");
        if (token && cJSON_IsString(token)) {
          strcpy(session_token, token->valuestring);
          save_session(username->valuestring, session_token);
          *logged_in = 1;
        } else {
          printf("Error: No session token received.\n");
        }
      }

    } else if (strcmp(action->valuestring, "REGISTER") == 0) {
      printf("Registration successful. You can now log in.\n");

    } else if (strcmp(action->valuestring, "ERROR") == 0) {
      // Handle errors sent from the server
      cJSON *message = cJSON_GetObjectItem(response, "message");
      if (message && cJSON_IsString(message)) {
        printf("Server error: %s\n", message->valuestring);
      } else {
        printf("Unknown error received.\n");
      }

    } else if (strcmp(action->valuestring, "MESSAGE_FROM_USER") == 0) {
      // Handle errors sent from the server
      cJSON *message = cJSON_GetObjectItem(response, "message");
      cJSON *sender = cJSON_GetObjectItem(response, "sender");
      if (message && cJSON_IsString(message)) {
        printf("Message from %s: %s\n", sender->valuestring,
               message->valuestring);
      } else {
        printf("Unknown error received.\n");
      }

    } else {
      printf("Unknown action: %s\n", action->valuestring);
    }
  } else {
    printf("Invalid or missing 'action' field in server response.\n");
  }

  // Clean up JSON object
  cJSON_Delete(response);
}
