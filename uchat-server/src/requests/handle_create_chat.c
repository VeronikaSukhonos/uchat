#include <uchat_server.h>

// Function to handle creating a private chat between two users
int handle_create_chat(sqlite3 *db, cJSON *json, Client *client,
                       Client clients[], int max_clients) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  if (logged_in(db, user_id) != 1) {
    return 1;
  }

  // Extract the other user's username from the JSON
  cJSON *other_user_json = cJSON_GetObjectItem(json, "username");
  if (!cJSON_IsString(other_user_json) ||
      other_user_json->valuestring == NULL) {
    fprintf(stderr, "Other user's username not found in JSON.\n");
    return 1;
  }
  const char *other_username = other_user_json->valuestring;

  // Get other user's ID
  int other_user_id = get_user_id(db, other_username);
  if (other_user_id == -1) {
    fprintf(stderr, "User not found: %s\n", other_username);
    return 1;
  }

  // Check if a chat already exists between these two users
  if (check_existing_chat(db, user_id, other_user_id) > 0) {
    printf("Chat already exists between users %d and %d.\n", user_id,
           other_user_id);
    return 0;
  }

  // Create a unique name for the private chat
  char chat_name[100];
  snprintf(chat_name, sizeof(chat_name), "%s-%s", client->username,
           other_username);

  // Create a new private chat and get its ID
  int chat_id = create_private_chat(db, chat_name);
  if (chat_id == -1) {
    return 1;
  }

  // Add both users as members of the chat
  if (add_chat_member(db, chat_id, user_id) != 0 ||
      add_chat_member(db, chat_id, other_user_id) != 0) {
    return 1;
  }

  printf("Chat created successfully between %s and %s with chat_id %d.\n",
         client->username, other_username, chat_id);

  // Build JSON response
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CREATE_CHAT");
  cJSON_AddStringToObject(response, "status", "SUCCESS");

  cJSON *chat_details = cJSON_CreateObject();
  cJSON_AddNumberToObject(chat_details, "chat_id", chat_id);
  cJSON_AddStringToObject(chat_details, "name", chat_name);
  cJSON_AddStringToObject(chat_details, "type", "private");

  // Include members
  cJSON *members_array = cJSON_CreateArray();
  cJSON_AddItemToArray(members_array, cJSON_CreateString(client->username));
  cJSON_AddItemToArray(members_array, cJSON_CreateString(other_username));
  cJSON_AddItemToObject(chat_details, "members", members_array);

  // Include messages
  cJSON_AddArrayToObject(chat_details, "messages");
  cJSON_AddItemToObject(response, "chat", chat_details);

  // Broadcast response to both users
  char *response_str = cJSON_Print(response);
  for (int i = 0; i < max_clients; i++) {
    if (clients[i].socket > 0 &&
        (strcmp(clients[i].username, client->username) == 0 ||
         strcmp(clients[i].username, other_username) == 0)) {
      send(clients[i].socket, response_str, strlen(response_str), 0);
    }
  }

  free(response_str);
  cJSON_Delete(response);
  return 0;
}