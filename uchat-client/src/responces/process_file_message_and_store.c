#include <stdio.h>
#include <stdlib.h>
#include <uchat.h>

void process_file_message_and_store(const char *json_response,
                                    AppData *app_data) {
  // Parse the JSON response
  cJSON *response = cJSON_Parse(json_response);
  if (!response) {
    fprintf(stderr, "Failed to parse JSON response.\n");
    return;
  }

  // Extract `chat_id`
  cJSON *chat_id_json = cJSON_GetObjectItem(response, "chat_id");
  if (!chat_id_json || !cJSON_IsNumber(chat_id_json)) {
    fprintf(stderr, "Failed to find `chat_id` in response.\n");
    cJSON_Delete(response);
    return;
  }
  int chat_id = chat_id_json->valueint;

  // Extract `message` object
  cJSON *message = cJSON_GetObjectItem(response, "message");
  if (!message) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  // Extract `file_name` and `voice_message`
  cJSON *message_id_json = cJSON_GetObjectItem(message, "message_id");
  cJSON *file_type_json = cJSON_GetObjectItem(message, "file_type");
  cJSON *file_path_json = cJSON_GetObjectItem(message, "file_path");
  cJSON *file_data_json = cJSON_GetObjectItem(message, "file_data");

  if (!file_path_json || !cJSON_IsString(file_path_json)) {
    fprintf(stderr, "Failed to find `file_name` in response.\n");
    cJSON_Delete(response);
    return;
  }

  if (!file_data_json || !cJSON_IsString(file_data_json)) {
    fprintf(stderr, "Failed to find `voice_message` in response.\n");
    cJSON_Delete(response);
    return;
  }

  const char *file_name = g_path_get_basename(file_path_json->valuestring);
  const char *encoded_file = file_data_json->valuestring;

  // Decode the Base64 voice message
  size_t decoded_size;
  unsigned char *decoded_file =
      base64_decode(encoded_file, strlen(encoded_file), &decoded_size);

  if (!decoded_file) {
    fprintf(stderr, "Failed to decode the voice message.\n");
    cJSON_Delete(response);
    return;
  }

  // Construct the file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "cache/chat_%d_%d.%s", chat_id,
           message_id_json->valueint, file_type_json->valuestring);

  // Save the decoded voice data to a .wav file
  FILE *file = fopen(file_path, "wb");
  if (!file) {
    perror("Failed to create voice file");
    g_free(decoded_file);
    cJSON_Delete(response);
    return;
  }

  fwrite(decoded_file, 1, decoded_size, file);
  fclose(file);
  g_free(decoded_file);

  // printf("Voice message saved to file: %s\n", file_path);
  cJSON_DeleteItemFromObject(message, "file_path");
  cJSON_DeleteItemFromObject(message, "file_type");
  cJSON_DeleteItemFromObject(message, "file_data");
  // Add the file path to the message object
  cJSON_AddStringToObject(message, "file_path", file_path);
  cJSON_AddStringToObject(message, "file_name", file_name);
  cJSON_AddStringToObject(message, "content", "");

  // Remove the `voice_message` field from the message object

  // Construct the encrypted JSON file path
  char chat_file_path[256];
  snprintf(chat_file_path, sizeof(chat_file_path), "cache/chat_%d.json",
           chat_id);

  // printf("Chat ID: %d\n", chat_id);
  // printf("Encrypted File Path: %s\n", chat_file_path);

  // Insert the message into the encrypted JSON file
  if (insert_message_into_chat(chat_file_path, message) == 0) {
    // printf("Voice message successfully stored in cache.\n");
    char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
    char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
    int chatId;

    if (read_chat_data_from_encrypted_json(chat_file_path, &chatId, name,
                                           chat_type, last_message, last_sender,
                                           last_time, unread) == 0) {
      // g_print("last sender: %s\n", last_sender);
      //  Create or update the chat button
      create_or_update_chat_button(app_data->main_page, chat_id, name,
                                   chat_type, last_message, last_sender,
                                   last_time, unread);
    }
  } else {
    fprintf(stderr, "Failed to store voice message in cache.\n");
  }
  is_user_scrolling = FALSE;
  MessageNode *msg_node = create_message_node(
      app_data->main_page, is_image(file_path) == 1 ? IMAGE : ANY_FILE, chat_id,
      message);
  create_message_button(app_data->main_page, msg_node);
  // g_print("Signal connected to play voice for message ID: %d\n",
  // msg_node->message->message_id);

  // Clean up
  cJSON_Delete(response);
}