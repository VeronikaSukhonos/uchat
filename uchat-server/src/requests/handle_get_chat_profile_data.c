#include <uchat_server.h>

void handle_get_chat_profile_data(sqlite3 *db, Client *client, cJSON *json,
                                  Client clients[], int max_clients) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return;
  }

  cJSON *chat_id_json = cJSON_GetObjectItem(json, "chat_id");
  if (!cJSON_IsNumber(chat_id_json)) {
    fprintf(stderr, "Invalid JSON format for sending message.\n");
    return;
  }
  int chat_id = chat_id_json->valueint;

  char type[15];
  get_chat_type(db, chat_id, type);
  cJSON *members = retrieve_chat_members(db, chat_id);

  for (int i = 0; i < cJSON_GetArraySize(members); i++) {
    cJSON *user_json = cJSON_GetArrayItem(members, i);
    char *username = cJSON_GetObjectItem(user_json, "username")->valuestring;
    int online_status = get_online_status(username, clients, max_clients);
    cJSON_AddStringToObject(user_json, "status",
                            online_status == 1 ? "online" : "offline");
    cJSON *user_data = get_user_profile_data(db, get_user_id(db, username));
    cJSON_AddStringToObject(
        user_json, "full_name",
        cJSON_GetObjectItem(user_data, "full_name")->valuestring);
    cJSON_AddStringToObject(
        user_json, "group",
        cJSON_GetObjectItem(user_data, "group")->valuestring);
    cJSON_AddStringToObject(
        user_json, "role", cJSON_GetObjectItem(user_data, "role")->valuestring);
  }

  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CHAT_PROFILE_DATA");
  cJSON_AddStringToObject(response, "type", type);
  cJSON_AddItemToObject(response, "members", members);

  send_json_responce_to_client(client, response);
}