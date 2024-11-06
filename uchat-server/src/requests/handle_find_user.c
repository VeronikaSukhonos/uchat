#include <uchat_server.h>

int handle_find_user(sqlite3 *db, cJSON *req, Client *client) {
  cJSON *username = cJSON_GetObjectItem(req, "username");

  if (strcmp(username->valuestring, client->username) == 0) {
    return 1;
  }

  int result = search_username(db, username->valuestring);
  if (result == 1) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "FIND_USER");
    cJSON_AddStringToObject(json, "status", "SUCCESS");
    cJSON_AddStringToObject(json, "username", username->valuestring);
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);
    send(client->socket, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    free(json_str);
    return 0;
  } else if (result == -1) {
    return 1;
  }
  return 1;
}
