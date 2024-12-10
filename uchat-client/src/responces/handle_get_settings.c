#include <uchat.h>
int handle_get_settings_response(cJSON *response, AppData *app_data) {
  cJSON *status = cJSON_GetObjectItem(response, "status");
  if (status && strcmp(status->valuestring, "SUCCESS") != 0) {
    fprintf(stderr, "Error: Server response status is not SUCCESS.\n");
    return 1;
  }

  cJSON *username_json = cJSON_GetObjectItem(response, "username");
  if (!username_json || !cJSON_IsString(username_json)) {
    fprintf(stderr, "Error: Missing or invalid username in the response.\n");
    return 1;
  }

  const char *new_username = username_json->valuestring;

  // Update the external username variable
  strncpy(username, new_username, sizeof(username) - 1);
  username[sizeof(username) - 1] = '\0'; // Ensure null termination

  return 0;
}
