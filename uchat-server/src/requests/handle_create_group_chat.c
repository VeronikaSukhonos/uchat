#include <uchat_server.h>

// Function to handle creating a group chat with multiple users
int handle_create_group_chat(sqlite3 *db, cJSON *json, Client *client,
                             Client clients[], int max_clients) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  if (logged_in(db, user_id) != 1) {
    return 1;
  }

  // Extract chat name
  cJSON *chat_name_json = cJSON_GetObjectItem(json, "chat_name");
  if (!cJSON_IsString(chat_name_json) || chat_name_json->valuestring == NULL) {
    fprintf(stderr, "Chat name not found in JSON.\n");
    return 1;
  }
  const char *chat_name = chat_name_json->valuestring;

  // Extract the array of usernames
  cJSON *usernames_json = cJSON_GetObjectItem(json, "usernames");
  if (!cJSON_IsArray(usernames_json)) {
    fprintf(stderr, "Usernames array not found in JSON.\n");
    return 1;
  }

  // Create the group chat
  int chat_id = create_private_group_chat(db, chat_name);
  if (chat_id == -1) {
    return 1;
  }

  // Add the creator as a member of the chat
  if (add_chat_member(db, chat_id, user_id) != 0) {
    fprintf(stderr, "Failed to add creator %s to chat %d.\n", client->username,
            chat_id);
    return 1;
  }

  // Add members to the group chat
  cJSON *username_json;
  cJSON *members_array = cJSON_CreateArray();
  cJSON_AddItemToArray(members_array, cJSON_CreateString(client->username));

  cJSON_ArrayForEach(username_json, usernames_json) {
    if (cJSON_IsString(username_json) && username_json->valuestring != NULL) {
      const char *username = username_json->valuestring;
      int member_user_id = get_user_id(db, username);

      if (member_user_id != -1 &&
          add_chat_member(db, chat_id, member_user_id) == 0) {
        cJSON_AddItemToArray(members_array, cJSON_CreateString(username));
      } else {
        fprintf(stderr, "Failed to add user %s to chat %d.\n", username,
                chat_id);
      }
    }
  }

  printf("Group chat '%s' created successfully with chat_id %d.\n", chat_name,
         chat_id);

  // Build JSON response
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "CREATE_CHAT");
  cJSON_AddStringToObject(response, "status", "SUCCESS");

  cJSON *chat_details = cJSON_CreateObject();
  cJSON_AddNumberToObject(chat_details, "chat_id", chat_id);
  cJSON_AddStringToObject(chat_details, "name", chat_name);
  cJSON_AddStringToObject(chat_details, "type", "private_group");
  cJSON_AddItemToObject(chat_details, "members", members_array);
  cJSON_AddArrayToObject(chat_details, "messages");

  cJSON_AddItemToObject(response, "chat", chat_details);

  // Broadcast response to all group members
  char *response_str = cJSON_Print(response);
  cJSON_ArrayForEach(username_json, members_array) {
    const char *username = cJSON_GetStringValue(username_json);
    if (username) {
      for (int i = 0; i < max_clients; i++) {
        if (clients[i].socket > 0 &&
            strcmp(clients[i].username, username) == 0) {
          send(clients[i].socket, response_str, strlen(response_str), 0);
        }
      }
    }
  }

  free(response_str);
  cJSON_Delete(response);
  return 0;
}