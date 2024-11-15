#include <openssl/evp.h>
#include <openssl/rand.h>
#include <uchat.h>

// Encrypt and save chat data to file
void handle_chat_list_response(cJSON *response, const char *cache_dir) {
  cJSON *chats = cJSON_GetObjectItem(response, "chats");
  if (!cJSON_IsArray(chats)) {
    printf("No chat list found in response.\n");
    return;
  }

  // Process each chat
  cJSON *chat;
  cJSON_ArrayForEach(chat, chats) {
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
    snprintf(chat_file_path, sizeof(chat_file_path), "%s/chat_%d.json",
             cache_dir, chat_id);

    // Save the chat data in encrypted form
    save_encrypted_chat_to_cache(chat_file_path, chat_data);
    cJSON_Delete(chat_data); // Clean up JSON data
  }
}