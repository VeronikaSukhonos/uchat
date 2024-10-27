#include <uchat.h>

char *build_json_register(const char *username, const char *email,
                          const char *password) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "REGISTER");
  cJSON_AddStringToObject(json, "username", username);
  cJSON_AddStringToObject(json, "email", email);
  cJSON_AddStringToObject(json, "password", password);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  return json_str;
}

// Function to build JSON for login
char *build_json_login(const char *username, const char *password,
                       const char *serial_number) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "LOGIN");
  cJSON_AddStringToObject(json, "username", username);
  cJSON_AddStringToObject(json, "serial_number", serial_number);
  cJSON_AddStringToObject(json, "password", password);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  return json_str;
}

// Function to build JSON for sending a message
char *build_json_message(const char *receiver, const char *message) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "SEND_MESSAGE_TO_USER");
  cJSON_AddStringToObject(json, "receiver", receiver);
  cJSON_AddStringToObject(json, "message", message);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  return json_str;
}

char *build_json_group_chat(const char *chat_name, char usernames[][50],
                            int num_users) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "CREATE_GROUP_CHAT");
  cJSON_AddStringToObject(json, "chat_name", chat_name);

  cJSON *usernames_array = cJSON_CreateArray();
  for (int i = 0; i < num_users; i++) {
    cJSON_AddItemToArray(usernames_array, cJSON_CreateString(usernames[i]));
  }
  cJSON_AddItemToObject(json, "usernames", usernames_array);

  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  return json_str;
}