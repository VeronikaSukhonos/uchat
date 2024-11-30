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
      cJSON_AddItemToObject(
          new_chat, "messages",
          cJSON_CreateArray()); // Start with an empty messages array

      // Save the new chat to the cache
      save_encrypted_chat_to_cache(file_path, new_chat);
      cJSON_Delete(new_chat);

      printf("New chat saved to cache: %d (%s)\n", chat_id, chat_name);
    } else {
      printf("Chat already exists in cache: %d (%s)\n", chat_id, chat_name);
    }
  }

  // Handle undelivered messages
  cJSON *message_group;
  for (message_group = undelivered_messages->child; message_group != NULL;
       message_group = message_group->next) {
    // Extract the chat ID from the key
    int chat_id = atoi(message_group->string);
    cJSON *messages = message_group;

    if (!cJSON_IsArray(messages)) {
      fprintf(stderr, "Invalid messages format for chat_id: %d\n", chat_id);
      continue;
    }

    if (cJSON_GetArraySize(messages) == 0) {
      printf("No messages for chat_id: %d\n", chat_id);
      continue;
    }

    // Process each message to handle voice message decoding
    cJSON *message;
    cJSON_ArrayForEach(message, messages) {
      // Extract `file_name` and `voice_message`
      cJSON *file_name_json = cJSON_GetObjectItem(message, "file_name");
      cJSON *voice_message_json = cJSON_GetObjectItem(message, "voice_message");

      if (!file_name_json || !cJSON_IsString(file_name_json)) {
        fprintf(stderr, "Failed to find `file_name` in message.\n");
        continue;
      }

      if (!voice_message_json || !cJSON_IsString(voice_message_json)) {
        fprintf(stderr, "Failed to find `voice_message` in message.\n");
        continue;
      }

      const char *file_name = file_name_json->valuestring;
      const char *encoded_voice = voice_message_json->valuestring;

      // Decode the Base64 voice message
      size_t decoded_size;
      unsigned char *decoded_voice =
          base64_decode(encoded_voice, strlen(encoded_voice), &decoded_size);

      if (!decoded_voice) {
        fprintf(stderr, "Failed to decode the voice message.\n");
        continue;
      }

      // Construct the file path
      char voice_file_path[256];
      snprintf(voice_file_path, sizeof(voice_file_path), "%s/%s", cache_dir,
               file_name);

      // Save the decoded voice data to a .wav file
      FILE *voice_file = fopen(voice_file_path, "wb");
      if (!voice_file) {
        perror("Failed to create voice file");
        g_free(decoded_voice);
        continue;
      }

      fwrite(decoded_voice, 1, decoded_size, voice_file);
      fclose(voice_file);
      g_free(decoded_voice);

      printf("Voice message saved to file: %s\n", voice_file_path);

      // Add the file path to the message object
      cJSON_AddStringToObject(message, "voice_file_path", voice_file_path);
      cJSON_AddStringToObject(message, "content", "");

      // Remove the `voice_message` and `file_name` fields from the message
      cJSON_DeleteItemFromObject(message, "voice_message");
      cJSON_DeleteItemFromObject(message, "file_name");
    }

    // Construct the file path for the cache
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/chat_%d.json", cache_dir,
             chat_id);

    // Merge undelivered messages into the cache
    merge_chat_data_with_server(file_path, messages, chat_id);
  }

  cJSON_Delete(response);
}