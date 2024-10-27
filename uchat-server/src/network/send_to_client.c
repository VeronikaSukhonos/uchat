#include <uchat_server.h>

// Send a message to a specific client
bool send_message_to_client(Client clients[], const char *message,
                            const char *receiver_username, int max_clients) {
  for (int i = 0; i < max_clients; i++) {
    if (clients[i].socket != 0 &&
        strcmp(clients[i].username, receiver_username) == 0) {
      send(clients[i].socket, message, strlen(message), 0);
      printf("Sent message to %s\n", receiver_username);
      return true;
    }
  }
  printf("User %s not found or not connected\n", receiver_username);
  return false;
}

void send_status_responce_to_client(Client *client, const char *action,
                                    const char *status) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", action);
  if (status != NULL) {
    cJSON_AddStringToObject(response, "status", status);
  }

  char *json_str = cJSON_Print(response);
  send(client->socket, json_str, strlen(json_str), 0);
  printf("Sent: %s\n", json_str);

  // Clean up
  cJSON_Delete(response);
  free(json_str);
}

void send_json_responce_to_client(Client *client, cJSON *json) {
  char *json_str = cJSON_Print(json);
  send(client->socket, json_str, strlen(json_str), 0);
  printf("Sent: %s\n", json_str);

  // Clean up
  cJSON_Delete(json);
  free(json_str);
}