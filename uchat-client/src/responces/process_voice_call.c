#include <uchat.h>

void process_voice_call_start(cJSON *response, AppData *app_data) {
  cJSON *caller_name_json = cJSON_GetObjectItem(response, "caller_name");
  cJSON *caller_port_json = cJSON_GetObjectItem(response, "caller_port");
  cJSON *caller_ip_json = cJSON_GetObjectItem(response, "caller_ip");

  char caller_name[50];
  int caller_port = caller_port_json->valueint;
  char caller_ip[INET_ADDRSTRLEN];

  strcpy(caller_name, caller_name_json->valuestring);
  strcpy(caller_ip, caller_ip_json->valuestring);

  if (in_call == 1) {
    g_print("Already in Call\n");
    return;
  }
  //   start_receive_pipeline();
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "ACCEPT_CALL");
  cJSON_AddStringToObject(json, "callee_name", username);
  cJSON_AddStringToObject(json, "caller_name", caller_name);
  cJSON_AddStringToObject(json, "caller_ipp", caller_ip);
  cJSON_AddNumberToObject(json, "caller_port", caller_port);
  cJSON_AddNumberToObject(json, "callee_port", receive_port);

  char *json_str = cJSON_Print(json);
  //   if (send(sock, json_str, strlen(json_str), 0) == -1) {
  //     stop_receive_pipeline();
  //     perror("Failed to send status to sender");
  //     return;
  //   }
  strcpy(app_data->main_page->accept_call, json_str);
  incoming = 1;
  create_voice_call_window(NULL, app_data->main_page, caller_name);
  //   start_send_pipeline(caller_ip, caller_port);
  //   in_call = 1;
  cJSON_free(json);
  free(json_str);
}

void process_voice_call_accept(cJSON *response, AppData *app_data) {
  cJSON *callee_name_json = cJSON_GetObjectItem(response, "callee_name");
  cJSON *callee_port_json = cJSON_GetObjectItem(response, "callee_port");
  cJSON *callee_ip_json = cJSON_GetObjectItem(response, "callee_ip");

  char callee_name[50];
  int callee_port = callee_port_json->valueint;
  char callee_ip[INET_ADDRSTRLEN];

  strcpy(callee_name, callee_name_json->valuestring);
  strcpy(callee_ip, callee_ip_json->valuestring);

  if (is_calling == 1) {
    start_receive_pipeline();
    start_send_pipeline(callee_ip, callee_port);
    is_calling = 0;
    in_call = 1;
    create_voice_call_window(NULL, app_data->main_page, "");
    gtk_label_set_text(GTK_LABEL(app_data->main_page->voice_call_window_label),
                       "In call");
  }
}

void process_voice_call_stop(cJSON *response, AppData *app_data) {
  if (in_call == 1) {
    stop_receive_pipeline();
    stop_send_pipeline();
    close_voice_call_window(app_data->main_page->voice_call_window,
                            app_data->main_page);
    g_print("Stopped voice call\n");
    in_call = 0;
  }
}