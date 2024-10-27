#include <uchat_server.h>

// Function to handle creating a private chat between two users
int handle_create_chat(sqlite3 *db, cJSON *json, Client *client) {
  int user_id =
      get_user_id(db, client->username); // Assuming client struct has username
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
    return 0; // Chat already exists
  }

  // Use a unique name for the private chat, like "user1-user2"
  char chat_name[100];
  snprintf(chat_name, sizeof(chat_name), "%s-%s", client->username,
           other_username);

  // Create a new private chat and get its ID
  int chat_id = create_private_chat(db, chat_name);
  if (chat_id == -1) {
    return 1; // Failed to create chat
  }

  // Add both users as members of the chat
  if (add_chat_member(db, chat_id, user_id) != 0 ||
      add_chat_member(db, chat_id, other_user_id) != 0) {
    return 1; // Failed to add members
  }

  printf("Chat created successfully between %s and %s with chat_id %d.\n",
         client->username, other_username, chat_id);

  return 0;
}
