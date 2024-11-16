#include <openssl/evp.h>
#include <openssl/rand.h>
#include <uchat.h>

void read_and_create_chat_button(const char *file_path,
                                 t_main_page_data *main_page) {
  // Variables to store the extracted chat data
  int chat_id = -1;
  char name[64] = {0};
  char chat_type[20] = {0};
  char last_message[1024] = {0};
  char last_sender[64] = {0};
  char last_time[32] = {0};
  char unread[16] = {0};

  // Read chat data from the encrypted JSON file
  if (read_chat_data_from_encrypted_json(file_path, &chat_id, name, chat_type,
                                         last_message, last_sender, last_time,
                                         unread) != 0) {
    fprintf(stderr, "Failed to read chat data from file: %s\n", file_path);
    return;
  }

  // Create a chat button using the extracted data
  new_chat_button_from_json(main_page, chat_id, name, chat_type, last_message,
                            last_sender, last_time, unread);

  t_chat_node *current_chat = main_page->chats;
  while (current_chat != NULL) {
    if (current_chat->chat.id == chat_id) {
      // Set this chat as the currently opened chat
      main_page->opened_chat = &current_chat->chat;
      g_print("Chat ID %d is now set as opened_chat.\n", chat_id);
      return;
    }
    current_chat = current_chat->next;
  }

  // If the chat_id is not found in the list, log a warning
  g_print("Warning: Chat ID %d not found in main_page->chats.\n", chat_id);
}

void save_single_chat_to_encrypted_cache(cJSON *chat, const char *cache_dir,
                                         AppData *app_data) {
  if (!chat) {
    fprintf(stderr, "Invalid chat data.\n");
    return;
  }

  int chat_id = cJSON_GetObjectItem(chat, "chat_id")->valueint;
  const char *chat_name = cJSON_GetObjectItem(chat, "name")->valuestring;
  const char *chat_type = cJSON_GetObjectItem(chat, "type")->valuestring;

  // Create a JSON object to store chat metadata, members, and messages
  cJSON *chat_data = cJSON_CreateObject();
  cJSON_AddNumberToObject(chat_data, "chat_id", chat_id);
  cJSON_AddStringToObject(chat_data, "name", chat_name);
  cJSON_AddStringToObject(chat_data, "type", chat_type);

  // Process members
  cJSON *members = cJSON_GetObjectItem(chat, "members");
  if (cJSON_IsArray(members)) {
    cJSON *members_array = cJSON_CreateArray();
    cJSON *member;
    cJSON_ArrayForEach(member, members) {
      if (cJSON_IsString(member)) {
        cJSON_AddItemToArray(members_array,
                             cJSON_CreateString(member->valuestring));
      }
    }
    cJSON_AddItemToObject(chat_data, "members", members_array);
  }

  // Process messages
  cJSON *messages = cJSON_GetObjectItem(chat, "messages");
  if (cJSON_IsArray(messages)) {
    cJSON *messages_array = cJSON_CreateArray();
    cJSON *message;
    cJSON_ArrayForEach(message, messages) {
      cJSON *json_message = cJSON_CreateObject();

      int message_id = cJSON_GetObjectItem(message, "message_id")->valueint;
      const char *sender =
          cJSON_GetObjectItem(message, "username")->valuestring;
      const char *content =
          cJSON_GetObjectItem(message, "content")->valuestring;
      const char *timestamp =
          cJSON_GetObjectItem(message, "timestamp")->valuestring;
      int read = cJSON_GetObjectItem(message, "read")->valueint;

      cJSON_AddNumberToObject(json_message, "message_id", message_id);
      cJSON_AddStringToObject(json_message, "sender", sender);
      cJSON_AddStringToObject(json_message, "content", content);
      cJSON_AddStringToObject(json_message, "timestamp", timestamp);
      cJSON_AddBoolToObject(json_message, "read", read);

      cJSON_AddItemToArray(messages_array, json_message);
    }
    cJSON_AddItemToObject(chat_data, "messages", messages_array);
  }

  // Create the file path for storing encrypted chat data
  char chat_file_path[256];
  snprintf(chat_file_path, sizeof(chat_file_path), "%s/chat_%d.json", cache_dir,
           chat_id);

  // Save the chat data in encrypted form
  save_encrypted_chat_to_cache(chat_file_path, chat_data);
  cJSON_Delete(chat_data); // Clean up JSON data

  // Create a chat button after saving
  read_and_create_chat_button(chat_file_path, app_data->main_page);
}