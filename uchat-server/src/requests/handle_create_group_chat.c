#include <uchat_server.h>

// Function to handle creating a group chat with multiple users
int handle_create_group_chat(sqlite3 *db, cJSON *json, Client *client) {
  int user_id = get_user_id(db, client->username);
  if (user_id == -1) {
    fprintf(stderr, "User not found: %s\n", client->username);
    return 1;
  }

  // Check if user is logged in
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
  int chat_id = create_private_group_chat(
      db, chat_name); // Create chat with `create_private_chat` helper function
  if (chat_id == -1) {
    return 1; // Failed to create chat
  }

  // Add the creator as a member of the chat
  if (add_chat_member(db, chat_id, user_id) != 0) {
    fprintf(stderr, "Failed to add creator %s to chat %d.\n", client->username,
            chat_id);
    return 1;
  }

  // Loop through the JSON array and add each user to the group chat
  cJSON *username_json;
  cJSON_ArrayForEach(username_json, usernames_json) {
    if (!cJSON_IsString(username_json) || username_json->valuestring == NULL) {
      fprintf(stderr, "Invalid username in array.\n");
      continue; // Skip invalid entries
    }
    const char *username = username_json->valuestring;

    int member_user_id = get_user_id(db, username);
    if (member_user_id == -1) {
      fprintf(stderr, "User not found: %s\n", username);
      continue; // Skip users not found
    }

    // Add each valid user as a member to the chat
    if (add_chat_member(db, chat_id, member_user_id) != 0) {
      fprintf(stderr, "Failed to add user %s to chat %d.\n", username, chat_id);
      continue; // Skip on failure to add
    }
  }

  printf("Group chat '%s' created successfully with chat_id %d.\n", chat_name,
         chat_id);
  return 0;
}
