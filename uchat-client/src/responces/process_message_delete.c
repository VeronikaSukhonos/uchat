#include <uchat.h>

void process_message_delete(const char *json_response, AppData *app_data) {
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

  // Extract `message_id`
  cJSON *message_id_json = cJSON_GetObjectItem(response, "message_id");
  if (!message_id_json || !cJSON_IsNumber(message_id_json)) {
    fprintf(stderr, "Failed to find `message_id` in response.\n");
    cJSON_Delete(response);
    return;
  }
  int message_id = message_id_json->valueint;

  // Construct the file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "cache/chat_%d.json", chat_id);

  // printf("Chat ID: %d\n", chat_id);
  // printf("Message ID: %d\n", message_id);
  // printf("File Path: %s\n", file_path);

  // Update the message status in the encrypted JSON file
  if (update_message_status_in_json(file_path, message_id, "deleted") == 0) {
    // printf("Message status successfully updated in cache.\n");

    // Update the message node in main_page->messages
    MessageNode *msg_node;
    for (msg_node = app_data->main_page->messages; msg_node != NULL;
         msg_node = msg_node->next) {
      if (msg_node->message->message_id == message_id) {
        // Update the message status
        msg_node->message->status = DELETED;

        // Update the UI: Remove content and update status label if applicable
        if (msg_node->message->button) {
          gtk_widget_destroy(msg_node->message->time_label);
          gtk_widget_destroy(msg_node->message->changed_label);
          gtk_widget_destroy(msg_node->message->seen_label);
          if (msg_node->message->content_type == VOICE) {
            gtk_widget_destroy(msg_node->message->voice_message_button);
            gtk_widget_set_visible(msg_node->message->message_label, 1);
          } else if (msg_node->message->content_type == ANY_FILE) {
            gtk_widget_destroy(msg_node->message->save_file_button);
          } else if (msg_node->message->content_type == IMAGE) {
            gtk_widget_destroy(msg_node->message->image_file);
            gtk_widget_set_visible(msg_node->message->message_label, 1);
          }
          gtk_label_set_text(GTK_LABEL(msg_node->message->message_label),
                             "Deleted message");
          gtk_style_context_add_class(
              gtk_widget_get_style_context(msg_node->message->message_label),
              "deleted-message");
          if (msg_node->message->menu != NULL)
            gtk_widget_destroy(msg_node->message->menu);
          msg_node->message->menu = NULL;
          if (msg_node->message->message_label) {
            gtk_label_set_text(GTK_LABEL(msg_node->message->message_label),
                               "Deleted message");
            gtk_style_context_add_class(
                gtk_widget_get_style_context(msg_node->message->message_label),
                "deleted-message");
          }
        }

        // g_print("Marked message ID %d as DELETED.\n", message_id);
        break;
      }
    }

    // Optionally update the chat button to reflect the change
    char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
    char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
    int chatId;

    if (read_chat_data_from_encrypted_json(file_path, &chatId, name, chat_type,
                                           last_message, last_sender, last_time,
                                           unread) == 0) {
      create_or_update_chat_button(app_data->main_page, chat_id, name,
                                   chat_type, last_message, last_sender,
                                   last_time, unread);
    }
  } else {
    fprintf(stderr, "Failed to update message status in cache.\n");
  }

  cJSON_Delete(response);
}
