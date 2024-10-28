#include <uchat.h>

int save_session(const char *username, const char *session_token) {
  FILE *file = fopen("session.txt", "w");
  if (file == NULL) {
    fprintf(stderr, "Failed to open session file for writing.\n");
    return -1;
  }

  fprintf(file, "%s\n%s\n", username, session_token);
  fclose(file);
  return 0;
}

int load_session(char *username, size_t username_size, char *session_token,
                 size_t token_size) {
  FILE *file = fopen("session.txt", "r");
  if (file == NULL) {
    fprintf(stderr, "No session file found. Please log in.\n");
    return -1;
  }

  if (fgets(username, username_size, file) == NULL ||
      fgets(session_token, token_size, file) == NULL) {
    fprintf(stderr, "Failed to read session data.\n");
    fclose(file);
    return -1;
  }

  // Remove newline characters
  username[strcspn(username, "\n")] = '\0';
  session_token[strcspn(session_token, "\n")] = '\0';

  fclose(file);
  return 0;
}

int check_session_on_server(int sock, const char *username,
                            const char *session_token,
                            const char *serial_number) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "CHECK_SESSION");
  cJSON_AddStringToObject(json, "username", username);
  cJSON_AddStringToObject(json, "serial_number", serial_number);
  cJSON_AddStringToObject(json, "session_token", session_token);
  char *json_str = cJSON_Print(json);

  // Send JSON data to the server
  send(sock, json_str, strlen(json_str), 0);
  char buffer[BUFFER_SIZE] = {0};

  int valread = read(sock, buffer, sizeof(buffer));
  if (valread > 0) {
    buffer[valread] = '\0';
    g_print("Server response: %s\n", buffer);
    cJSON *response = cJSON_Parse(buffer);
    if (response == NULL) {
      g_print("Failed to parse server response.\n");
      return 0;
    }

    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Session valid.\n");
      return 1;
    }
  } else if (valread == 0) {
    g_print("Server disconnected\n");
  }
  return 0;
}

int delete_session() {
  if (remove("session.txt") == 0) {
    printf("Session deleted successfully.\n");
    return 0;
  } else {
    fprintf(stderr, "Failed to delete session file.\n");
    return -1;
  }
}
