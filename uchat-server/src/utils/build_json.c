#include <uchat_server.h>

cJSON *build_json_login(const char *username, const char *token) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "LOGIN");
  cJSON_AddStringToObject(json, "status", "SUCCESS");
  cJSON_AddStringToObject(json, "username", username);
  cJSON_AddStringToObject(json, "session_token", token);
  return json;
}
