#include <uchat_server.h>

// Function to check if the user is logged in
int is_user_logged_in(sqlite3 *db, int user_id) {
  return logged_in(db,
                   user_id); // Assuming logged_in() checks user's login status
}

// Function to handle retrieving the chat list for a use

int handle_get_chat_list(sqlite3 *db, Client *client) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  // Check if user is logged in
  if (!is_user_logged_in(db, user_id)) {
    fprintf(stderr, "User is not logged in.\n");
    return 1;
  }

  // Create JSON response
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CHAT_LIST");

  // Retrieve chat list from database
  cJSON *chats = retrieve_chat_list(db, user_id);
  if (chats == NULL) {
    cJSON_Delete(response);
    return 1;
  }
  cJSON_AddItemToObject(response, "chats", chats);

  // Send response to client
  char *response_str = cJSON_Print(response);
  printf("Sent: %s", response_str);
  send(client->socket, response_str, strlen(response_str), 0);
  free(response_str);
  cJSON_Delete(response);

  return 0;
}