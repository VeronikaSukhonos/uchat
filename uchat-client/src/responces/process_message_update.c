#include <uchat.h>

void process_message_update(const char *json_response, AppData *app_data) {
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

  // printf("Chat ID: %d\n", chat_id);
  // printf("File Path: %s\n", file_path);

  // Extract `message` object
  cJSON *message = cJSON_GetObjectItem(response, "message");
  if (!message) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  cJSON *message_id = cJSON_GetObjectItem(message, "message_id");
  if (!message_id) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }
  cJSON *content = cJSON_GetObjectItem(message, "content");
  if (!content) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  // At this point, you can insert the message into the encrypted file
  // Use your function to insert the message at the beginning of the file
  if (update_message_in_chat(file_path, message_id->valueint,
                             content->valuestring) == 0) {
    // printf("Message successfully stored in cache.\n");
    char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
    char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
    int chatId;

    if (read_chat_data_from_encrypted_json(file_path, &chatId, name, chat_type,
                                           last_message, last_sender, last_time,
                                           unread) == 0) {
      // g_print("last sender: %s\n", last_sender);
      //  Create or update the chat button
      create_or_update_chat_button(app_data->main_page, chat_id, name,
                                   chat_type, last_message, last_sender,
                                   last_time, unread);
    }
  } else {
    fprintf(stderr, "Failed to store message in cache.\n");
  }

  //   MessageNode *msg_node =
  //       create_message_node(app_data->main_page, TEXT, chat_id, message);
  //   create_message_button(app_data->main_page, msg_node);

  // Clean up

  gtk_label_set_text(GTK_LABEL(app_data->main_page->opened_chat
                                   ->changing_message->message->changed_label),
                     "Edited");
  // g_print("message changed from \"%s\" to \"%s\"\n",
  // app_data->main_page->opened_chat->changing_message->message->content,
  // content->valuestring);
  app_data->main_page->opened_chat->changing_message->message->status =
      MODIFIED;
  strcpy(app_data->main_page->opened_chat->changing_message->message->content,
         content->valuestring);
  gtk_label_set_text(GTK_LABEL(app_data->main_page->opened_chat
                                   ->changing_message->message->message_label),
                     content->valuestring);
  app_data->main_page->opened_chat->changing_message = NULL;

  cJSON_Delete(response);
}

void process_message_update_from_chat(const char *json_response,
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

  // Construct the file path
  char file_path[256];
  snprintf(file_path, sizeof(file_path), "cache/chat_%d.json", chat_id);

  // printf("Chat ID: %d\n", chat_id);
  // printf("File Path: %s\n", file_path);

  // Extract `message` object
  cJSON *message = cJSON_GetObjectItem(response, "message");
  if (!message) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  cJSON *message_id = cJSON_GetObjectItem(message, "message_id");
  if (!message_id) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }
  cJSON *content = cJSON_GetObjectItem(message, "content");
  if (!content) {
    fprintf(stderr, "Failed to find `message` object in response.\n");
    cJSON_Delete(response);
    return;
  }

  // At this point, you can insert the message into the encrypted file
  // Use your function to insert the message at the beginning of the file
  if (update_message_in_chat(file_path, message_id->valueint,
                             content->valuestring) == 0) {
    // printf("Message successfully stored in cache.\n");
    char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
    char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
    int chatId;

    if (read_chat_data_from_encrypted_json(file_path, &chatId, name, chat_type,
                                           last_message, last_sender, last_time,
                                           unread) == 0) {
      // g_print("last sender: %s\n", last_sender);
      //  Create or update the chat button
      create_or_update_chat_button(app_data->main_page, chat_id, name,
                                   chat_type, last_message, last_sender,
                                   last_time, unread);
    }
  } else {
    fprintf(stderr, "Failed to store message in cache.\n");
  }

  //   MessageNode *msg_node =
  //       create_message_node(app_data->main_page, TEXT, chat_id, message);
  //   create_message_button(app_data->main_page, msg_node);

  // Clean up

  //   gtk_label_set_text(GTK_LABEL(app_data->main_page->opened_chat
  //                                    ->changing_message->message->changed_label),
  //                      "Edited");
  //   //g_print("message changed from \"%s\" to \"%s\"\n",
  //           app_data->main_page->opened_chat->changing_message->message->content,
  //           content->valuestring);
  //   app_data->main_page->opened_chat->changing_message->message->status =
  //       MODIFIED;
  //   strcpy(app_data->main_page->opened_chat->changing_message->message->content,
  //          content->valuestring);
  //   gtk_label_set_text(GTK_LABEL(app_data->main_page->opened_chat
  //                                    ->changing_message->message->message_label),
  //                      content->valuestring);
  //   app_data->main_page->opened_chat->changing_message = NULL;

  MessageNode *msg_node;
  for (msg_node = app_data->main_page->messages; msg_node != NULL;
       msg_node = msg_node->next) {
    if (msg_node->message->message_id == message_id->valueint) {
      // Update the message content and status
      strcpy(msg_node->message->content, content->valuestring);
      msg_node->message->status = MODIFIED;

      // Update the message button if it exists
      if (msg_node->message->button) {
        if (msg_node->message->message_label) {
          gtk_label_set_text(GTK_LABEL(msg_node->message->message_label),
                             content->valuestring);
        }

        if (msg_node->message->changed_label) {
          gtk_label_set_text(GTK_LABEL(msg_node->message->changed_label),
                             "Edited");
        }
      }

      // g_print("Updated message ID %d to new content: \"%s\"\n",
      // message_id->valueint, content->valuestring);
      break;
    }
  }

  cJSON_Delete(response);
}
