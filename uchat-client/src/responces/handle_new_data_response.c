#include <uchat.h>

void handle_new_data_response(const char *response_data,
                              const char *cache_dir) {
  // Parse the server response
  cJSON *response = cJSON_Parse(response_data);
  if (!response) {
    fprintf(stderr, "Failed to parse server response.\n");
    return;
  }

  // Extract missing chats and undelivered messages
  cJSON *missing_chats = cJSON_GetObjectItem(response, "missing_chats");
  cJSON *undelivered_messages =
      cJSON_GetObjectItem(response, "undelivered_messages");

  if (!cJSON_IsArray(missing_chats) || !cJSON_IsObject(undelivered_messages)) {
    fprintf(stderr, "Invalid server response format.\n");
    cJSON_Delete(response);
    return;
  }

  // Handle missing chats
  cJSON *chat;
  cJSON_ArrayForEach(chat, missing_chats) {
    // Extract chat data from the server's response
    int chat_id = cJSON_GetObjectItem(chat, "chat_id")->valueint;
    const char *chat_name = cJSON_GetObjectItem(chat, "name")->valuestring;
    const char *chat_type = cJSON_GetObjectItem(chat, "type")->valuestring;
    cJSON *members = cJSON_GetObjectItem(chat, "members");

    // Construct the file path for the cache
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/chat_%d.json", cache_dir,
             chat_id);

    // Check if the chat already exists in the cache
    if (access(file_path, F_OK) != 0) {
      // Chat does not exist, create a new chat structure with the provided data
      cJSON *new_chat = cJSON_CreateObject();
      cJSON_AddNumberToObject(new_chat, "chat_id", chat_id);
      cJSON_AddStringToObject(new_chat, "name",
                              chat_name ? chat_name : "Unknown Chat");
      cJSON_AddStringToObject(new_chat, "type",
                              chat_type ? chat_type : "unknown");
      cJSON_AddItemToObject(new_chat, "members", cJSON_Duplicate(members, 1));
      cJSON_AddItemToObject(new_chat, "messages", cJSON_CreateArray());

      save_encrypted_chat_to_cache(file_path, new_chat);
      cJSON_Delete(new_chat);

      // printf("New chat saved to cache: %d (%s)\n", chat_id, chat_name);
    } else {
      // printf("Chat already exists in cache: %d (%s)\n", chat_id, chat_name);
    }
  }

  // Handle undelivered messages
  cJSON *message_group;
  for (message_group = undelivered_messages->child; message_group != NULL;
       message_group = message_group->next) {
    int chat_id = atoi(message_group->string);
    cJSON *messages = message_group;

    if (!cJSON_IsArray(messages)) {
      fprintf(stderr, "Invalid messages format for chat_id: %d\n", chat_id);
      continue;
    }

    if (cJSON_GetArraySize(messages) == 0) {
      // printf("No messages for chat_id: %d\n", chat_id);
      continue;
    }

    // Process each message
    cJSON *message;
    cJSON_ArrayForEach(message, messages) {
      const char *type = cJSON_GetObjectItem(message, "type")->valuestring;

      if (strcmp(type, "voice") == 0) {
        // Process voice message
        cJSON *voice_message_json =
            cJSON_GetObjectItem(message, "voice_message");
        const char *file_name =
            cJSON_GetObjectItem(message, "file_name")->valuestring;

        if (voice_message_json && file_name) {
          size_t decoded_size;
          unsigned char *decoded_voice = base64_decode(
              voice_message_json->valuestring,
              strlen(voice_message_json->valuestring), &decoded_size);

          if (decoded_voice) {
            char voice_file_path[256];
            snprintf(voice_file_path, sizeof(voice_file_path), "%s/%s",
                     cache_dir, file_name);

            FILE *voice_file = fopen(voice_file_path, "wb");
            if (voice_file) {
              fwrite(decoded_voice, 1, decoded_size, voice_file);
              fclose(voice_file);

              cJSON_AddStringToObject(message, "voice_file_path",
                                      voice_file_path);
              // g_print("Voice message saved to: %s\n", voice_file_path);
            } else {
              perror("Failed to create voice file");
            }

            g_free(decoded_voice);
          } else {
            fprintf(stderr, "Failed to decode voice message for chat_id: %d\n",
                    chat_id);
          }
        }
      } else if (strcmp(type, "file") == 0) {
        // Process file message
        cJSON *message_id_json = cJSON_GetObjectItem(message, "message_id");
        cJSON *file_type_json = cJSON_GetObjectItem(message, "file_type");
        cJSON *file_path_json = cJSON_GetObjectItem(message, "file_path");
        cJSON *file_data_json = cJSON_GetObjectItem(message, "file_data");

        if (file_path_json && file_data_json) {
          size_t decoded_size;
          unsigned char *decoded_file =
              base64_decode(file_data_json->valuestring,
                            strlen(file_data_json->valuestring), &decoded_size);

          if (decoded_file) {
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s/chat_%d_%d.%s",
                     cache_dir, chat_id, message_id_json->valueint,
                     file_type_json->valuestring);

            // char file_path[256];
            // snprintf(file_path, sizeof(file_path), "%s/chat_%d_%d.%s",
            //          cache_dir, chat_id, message_id_json->valueint,
            //          file_type_json->valuestring);
            char *file_name = g_path_get_basename(file_path_json->valuestring);

            FILE *file = fopen(file_path, "wb");
            if (file) {
              fwrite(decoded_file, 1, decoded_size, file);
              fclose(file);

              cJSON_DeleteItemFromObject(message, "file_path");
              cJSON_DeleteItemFromObject(message, "file_type");
              cJSON_DeleteItemFromObject(message, "file_data");

              cJSON_AddStringToObject(message, "file_path", file_path);
              cJSON_AddStringToObject(message, "file_name", file_name);
              // g_print("File saved to: %s\n", file_path);
            } else {
              perror("Failed to create file");
            }

            g_free(decoded_file);
          } else {
            fprintf(stderr, "Failed to decode file for chat_id: %d\n", chat_id);
          }
        }
      }
    }

    // Save updated messages to cache
    char chat_file_path[256];
    snprintf(chat_file_path, sizeof(chat_file_path), "%s/chat_%d.json",
             cache_dir, chat_id);
    merge_chat_data_with_server(chat_file_path, messages, chat_id);
  }

  cJSON_Delete(response);
}