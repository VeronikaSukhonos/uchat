#include <uchat.h>

// Function to send request for new data (undelivered messages)
void send_get_new_data_request(int sock, cJSON *cached_chats) {
  cJSON *request = cJSON_CreateObject();
  cJSON_AddStringToObject(request, "action", "GET_NEW_DATA");

  // Add the list of cached chat IDs
  cJSON_AddItemToObject(request, "cached_chats", cached_chats);

  char *request_str = cJSON_Print(request);
  send(sock, request_str, strlen(request_str), 0);
  // g_print("Sent: %s\n", request_str);
  g_free(request_str);
  cJSON_Delete(request);
}

// Function to send a full chat list request if no cache exists
void send_get_chat_list_request(int sock) {
  cJSON *request = cJSON_CreateObject();
  cJSON_AddStringToObject(request, "action", "GET_CHAT_LIST");
  char *request_str = cJSON_Print(request);
  send(sock, request_str, strlen(request_str), 0);
  // g_print("Sent: %s\n", request_str);
  g_free(request_str);
  cJSON_Delete(request);
}

// Function to check if cache exists and decide what to send to the server
void sync_chat_list_with_server(int sock, const char *cache_dir) {
  // Create a JSON array for storing cached chat IDs
  cJSON *cached_chats = cJSON_CreateArray();

  // Check if the cache directory exists
  if (access(cache_dir, F_OK) != -1) {
    // Cache exists, check for undelivered messages
    // g_print("Cache directory exists, checking for undelivered
    // messages...\n");

    // Retrieve the list of chat IDs in the cache
    get_cached_chat_ids(cache_dir, cached_chats);

    // Send request for new data (undelivered messages) along with cached chat
    // list
    send_get_new_data_request(sock, cached_chats);
  } else {
    // Cache doesn't exist, send a request to get the full chat list from the
    // server
    // g_print("Cache directory doesn't exist, requesting full chat list...\n");
    send_get_chat_list_request(sock);
  }
}