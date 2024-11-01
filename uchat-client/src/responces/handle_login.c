#include <uchat.h>

int handle_login_response(cJSON *response) {
  cJSON *status = cJSON_GetObjectItem(response, "status");
  if (strcmp(status->valuestring, "SUCCESS") == 0) {
    char session_token[33];
    cJSON *username = cJSON_GetObjectItem(response, "username");

    // Get the session token from the response
    cJSON *token = cJSON_GetObjectItem(response, "session_token");
    if (token && cJSON_IsString(token)) {
      strcpy(session_token, token->valuestring);
      save_session(username->valuestring, session_token);
      return 0;
    }
  }
  return 1;
}