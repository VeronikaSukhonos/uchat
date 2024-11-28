#include <uchat_server.h>

void handle_call_forward(Client clients[], Client *client, cJSON *json,
                         int max_clients) {
  cJSON *callee_name = cJSON_GetObjectItem(json, "callee_name");
  cJSON *caller_port = cJSON_GetObjectItem(json, "caller_port");
  if (!callee_name || !cJSON_IsString(callee_name) || !caller_port ||
      !cJSON_IsNumber(caller_port)) {
    fprintf(stderr, "Invalid or missing calle_name or caller_port in JSON.\n");
    return;
  }
  int target_sock = -1;
  for (int j = 0; j < max_clients; j++) {
    if (strcmp(clients[j].username, callee_name->valuestring) == 0) {
      target_sock = clients[j].socket;
      break;
    }
  }

  if (target_sock == -1) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "action", "CALL_OFFLINE");
    char *response_str = cJSON_Print(response);
    if (send(client->socket, response_str, strlen(response_str), 0) == -1) {
      perror("Failed to send status to sender");
    }
    cJSON_free(response);
    free(response_str);
    return;
  }

  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CALL");
  cJSON_AddNumberToObject(response, "caller_port", caller_port->valueint);
  cJSON_AddStringToObject(response, "caller_name", client->username);
  cJSON_AddStringToObject(response, "caller_ip", client->ip_address);

  char *response_str = cJSON_Print(response);
  cJSON_free(response);
  if (send(target_sock, response_str, strlen(response_str), 0) == -1) {
    perror("Failed to send status to sender");
  }

  free(response_str);
}
void handle_accept_call_forward(Client clients[], Client *client, cJSON *json,
                                int max_clients) {
  cJSON *callee_name = cJSON_GetObjectItem(json, "callee_name");
  cJSON *caller_name = cJSON_GetObjectItem(json, "caller_name");
  cJSON *callee_port = cJSON_GetObjectItem(json, "callee_port");
  if (!callee_name || !cJSON_IsString(callee_name) || !callee_port ||
      !cJSON_IsNumber(callee_port) || !caller_name ||
      !cJSON_IsString(caller_name)) {
    fprintf(stderr, "Invalid or missing calle_name or caller_port in JSON.\n");
    return;
  }
  int target_sock = -1;
  for (int j = 0; j < max_clients; j++) {
    if (strcmp(clients[j].username, caller_name->valuestring) == 0) {
      target_sock = clients[j].socket;
      break;
    }
  }

  if (target_sock == -1) {
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "action", "CALL_OFFLINE");
    char *response_str = cJSON_Print(response);
    cJSON_free(response);
    if (send(client->socket, response_str, strlen(response_str), 0) == -1) {
      perror("Failed to send status to sender");
    }
    cJSON_free(response);
    free(response_str);
    return;
  }
  strcpy(client->in_call_with, caller_name->valuestring);
  for (int j = 0; j < max_clients; j++) {
    if (clients[j].socket == target_sock) {
      strcpy(clients[j].in_call_with, client->username);
      break;
    }
  }

  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "ACCEPT_CALL");
  cJSON_AddNumberToObject(response, "callee_port", callee_port->valueint);
  cJSON_AddStringToObject(response, "callee_name", client->username);
  cJSON_AddStringToObject(response, "callee_ip", client->ip_address);

  char *response_str = cJSON_Print(response);
  cJSON_free(response);
  if (send(target_sock, response_str, strlen(response_str), 0) == -1) {
    perror("Failed to send status to sender");
  }

  free(response_str);
}
void handle_stop_call_forward(Client clients[], Client *client, cJSON *json,
                              int max_clients) {
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "STOP_CALL");
  char *response_str = cJSON_Print(response);
  int target_sock = -1;
  for (int j = 0; j < max_clients; j++) {
    if (strcmp(clients[j].in_call_with, client->username) == 0) {
      send(client->socket, response_str, strlen(response_str), 0);
      send(clients[j].socket, response_str, strlen(response_str), 0);
      strcpy(client->in_call_with, "");
      strcpy(clients[j].in_call_with, "");
      printf("Send stop to %s and %s\n", client->username, clients[j].username);
      break;
    }
  }
  cJSON_free(response);
  free(response_str);
}