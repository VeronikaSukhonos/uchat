#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <string.h>
#include <uchat.h>

// Function to handle chat list response and save data to cache
void handle_chat_list_response(cJSON *response, const char *cache_dir) {
  cJSON *chats = cJSON_GetObjectItem(response, "chats");
  if (!cJSON_IsArray(chats)) {
    // printf("No chat list found in response.\n");
    return;
  }

  // Process each chat
  cJSON *chat;
  cJSON_ArrayForEach(chat, chats) {
    // Validate essential fields
    cJSON *chat_id_json = cJSON_GetObjectItem(chat, "chat_id");
    cJSON *chat_name_json = cJSON_GetObjectItem(chat, "name");
    cJSON *chat_type_json = cJSON_GetObjectItem(chat, "type");

    if (!cJSON_IsNumber(chat_id_json) || !cJSON_IsString(chat_name_json) ||
        !cJSON_IsString(chat_type_json)) {
      fprintf(stderr, "Invalid or missing essential fields in chat data.\n");
      continue;
    }

    int chat_id = chat_id_json->valueint;
    const char *chat_name = chat_name_json->valuestring;
    const char *chat_type = chat_type_json->valuestring;

    // Create a JSON object to store chat metadata, members, and messages
    cJSON *chat_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(chat_data, "chat_id", chat_id);
    cJSON_AddStringToObject(chat_data, "name", chat_name);
    cJSON_AddStringToObject(chat_data, "type", chat_type);

    // Process members
    cJSON *members = cJSON_GetObjectItem(chat, "members");
    cJSON *members_array = cJSON_CreateArray();
    if (cJSON_IsArray(members)) {
      cJSON *member;
      cJSON_ArrayForEach(member, members) {
        if (cJSON_IsObject(member)) {
          cJSON_AddItemToArray(members_array, cJSON_Duplicate(member, 1));
        }
      }
    }
    cJSON_AddItemToObject(chat_data, "members", members_array);

    // Process messages
    cJSON *messages = cJSON_GetObjectItem(chat, "messages");
    cJSON *messages_array = cJSON_CreateArray();
    if (cJSON_IsArray(messages)) {
      cJSON *message;
      cJSON_ArrayForEach(message, messages) {
        // Validate required fields in the message
        cJSON *message_id_json = cJSON_GetObjectItem(message, "message_id");
        cJSON *username_json = cJSON_GetObjectItem(message, "username");
        cJSON *content_json = cJSON_GetObjectItem(message, "content");
        cJSON *timestamp_json = cJSON_GetObjectItem(message, "timestamp");
        cJSON *content_type_json = cJSON_GetObjectItem(message, "type");
        cJSON *read_json = cJSON_GetObjectItem(message, "read");
        cJSON *sender_id_json = cJSON_GetObjectItem(message, "sender_id");
        cJSON *status_json = cJSON_GetObjectItem(message, "status");

        if (!cJSON_IsNumber(message_id_json) ||
            !cJSON_IsString(username_json) || !cJSON_IsString(content_json) ||
            !cJSON_IsString(timestamp_json) ||
            !cJSON_IsString(content_type_json) ||
            !cJSON_IsNumber(sender_id_json)) {
          fprintf(stderr, "Invalid or missing fields in message data.\n");
          continue;
        }

        // Create a message object
        cJSON *json_message = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_message, "message_id",
                                message_id_json->valueint);
        cJSON_AddNumberToObject(json_message, "sender_id",
                                sender_id_json->valueint);
        cJSON_AddStringToObject(json_message, "username",
                                username_json->valuestring);
        cJSON_AddStringToObject(json_message, "content",
                                content_json->valuestring);
        cJSON_AddStringToObject(json_message, "type",
                                content_type_json->valuestring);
        cJSON_AddStringToObject(json_message, "timestamp",
                                timestamp_json->valuestring);
        cJSON_AddBoolToObject(json_message, "read", cJSON_IsTrue(read_json));
        cJSON_AddStringToObject(json_message, "status",
                                status_json->valuestring);

        // Handle voice messages
        if (strcmp(content_type_json->valuestring, "voice") == 0) {
          // Decode Base64 voice message content
          cJSON *voice_json = cJSON_GetObjectItem(message, "voice_message");
          size_t decoded_size;
          unsigned char *decoded_data =
              base64_decode(voice_json->valuestring,
                            strlen(voice_json->valuestring), &decoded_size);
          if (decoded_data) {
            // Create a file path for the voice message

            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s/chat_%d_%d_vmsg.wav",
                     cache_dir, chat_id, message_id_json->valueint);

            // Write the decoded voice message to a file
            FILE *voice_file = fopen(file_path, "wb");
            if (voice_file) {
              fwrite(decoded_data, 1, decoded_size, voice_file);
              fclose(voice_file);

              // Add the file path to the JSON message object
              cJSON_AddStringToObject(json_message, "voice_file_path",
                                      file_path);
              // g_print("success to write voice message to file: %s\n",
              // file_path);
            } else {
              fprintf(stderr, "Failed to write voice message to file: %s\n",
                      file_path);
            }

            g_free(decoded_data);
          } else {
            fprintf(stderr,
                    "Failed to decode voice message for message_id: %d\n",
                    message_id_json->valueint);
          }
        }
        if (strcmp(content_type_json->valuestring, "file") == 0) {
          cJSON *file_path_json = cJSON_GetObjectItem(message, "file_path");
          cJSON *file_type_json = cJSON_GetObjectItem(message, "file_type");
          cJSON *file_data_json = cJSON_GetObjectItem(message, "file_data");

          size_t decoded_size;
          unsigned char *decoded_data =
              base64_decode(file_data_json->valuestring,
                            strlen(file_data_json->valuestring), &decoded_size);

          char file_path[256];
          snprintf(file_path, sizeof(file_path), "%s/chat_%d_%d.%s", cache_dir,
                   chat_id, message_id_json->valueint,
                   file_type_json->valuestring);
          char *file_name = g_path_get_basename(file_path_json->valuestring);

          // Write the decoded voice message to a file
          FILE *file = fopen(file_path, "wb");
          if (file) {
            fwrite(decoded_data, 1, decoded_size, file);
            fclose(file);

            // Add the file path to the JSON message object
            cJSON_AddStringToObject(json_message, "file_path", file_path);
            cJSON_AddStringToObject(json_message, "file_name", file_name);
            // g_print("success to write voice message to file: %s\n",
            // file_path);
          } else {
            fprintf(stderr, "Failed to write voice message to file: %s\n",
                    file_path);
          }
        }

        // Remove the voice_message field after processing
        cJSON_DeleteItemFromObject(message, "voice_message");
        cJSON_DeleteItemFromObject(message, "file_name");

        cJSON_AddItemToArray(messages_array, json_message);
      }
    }
    cJSON_AddItemToObject(chat_data, "messages", messages_array);

    // Create the file path for storing encrypted chat data
    char chat_file_path[256];
    snprintf(chat_file_path, sizeof(chat_file_path), "%s/chat_%d.json",
             cache_dir, chat_id);

    // Save the chat data in encrypted form
    save_encrypted_chat_to_cache(chat_file_path, chat_data);
    cJSON_Delete(chat_data); // Clean up JSON data
  }
}