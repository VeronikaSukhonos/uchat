#include <uchat.h>

void process_message_and_store(const char *json_response, AppData *app_data) {
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

  // Construct the file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "cache/chat_%d.json", chat_id);

  printf("Chat ID: %d\n", chat_id);
  printf("File Path: %s\n", file_path);

  // Extract `message` object
  cJSON *message = cJSON_GetObjectItem(response, "message");
  if (!message) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  // At this point, you can insert the message into the encrypted file
  // Use your function to insert the message at the beginning of the file
  if (insert_message_into_chat(file_path, message) == 0) {
    printf("Message successfully stored in cache.\n");
    char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
    char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
    int chatId;

    if (read_chat_data_from_encrypted_json(file_path, &chatId, name, chat_type,
                                           last_message, last_sender, last_time,
                                           unread) == 0) {
      g_print("last sender: %s\n", last_sender);
      // Create or update the chat button
      create_or_update_chat_button(app_data->main_page, chat_id, name,
                                   chat_type, last_message, last_sender,
                                   last_time, unread);
    }
  } else {
    fprintf(stderr, "Failed to store message in cache.\n");
  }
  is_user_scrolling = FALSE;
  MessageNode *msg_node =
      create_message_node(app_data->main_page, TEXT, chat_id, message);
  create_message_button(app_data->main_page, msg_node);

  // Clean up
  cJSON_Delete(response);
}