#include <uchat.h>

#define INITIAL_BUFFER_SIZE                                                    \
  4096 // Adjust as necessary for expected response size

char *receive_large_json(int socket_fd) {
  size_t buffer_size = INITIAL_BUFFER_SIZE;
  char *buffer = malloc(buffer_size);
  if (!buffer) {
    perror("malloc failed");
    return NULL;
  }

  size_t received = 0;
  ssize_t bytes;
  int end_detected = 0;

  while (!end_detected) {
    // Set up a timeout using select
    struct timeval timeout;
    timeout.tv_sec = 5; // 5 seconds timeout
    timeout.tv_usec = 0;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);

    int activity = select(socket_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (activity < 0) {
      perror("select failed");
      free(buffer);
      return NULL;
    } else if (activity == 0) {
      // Timeout occurred
      fprintf(stderr, "recv timed out.\n");
      free(buffer);
      return NULL;
    }

    // If data is ready to be read
    bytes = recv(socket_fd, buffer + received, buffer_size - received, 0);
    if (bytes < 0) {
      perror("recv failed");
      free(buffer);
      return NULL;
    } else if (bytes == 0) {
      // Connection closed by the server
      break;
    }

    received += bytes;

    // If buffer is full, resize it
    if (received >= buffer_size) {
      buffer_size *= 2; // Double the buffer size
      char *temp = realloc(buffer, buffer_size);
      if (!temp) {
        perror("realloc failed");
        free(buffer);
        return NULL;
      }
      buffer = temp;
    }

    // Check if we've reached the end of a JSON object
    if (received >= 2 && buffer[received - 1] == '}' &&
        buffer[received - 2] != '\\') {
      end_detected = 1;
    }
  }

  buffer[received] = '\0'; // Null-terminate the JSON data
  return buffer;
}
// Function to read and handle server responses
int handle_response(int sock, int *logged_in, AppData *app_data) {
  char *buffer = receive_large_json(sock);
  if (!buffer) {
    g_print("Failed to receive server response.\n");
    *logged_in = 0;
    return -1;
  }

  g_print("Buffer: %s\n", buffer);

  cJSON *response = cJSON_Parse(buffer);
  free(buffer); // Free buffer after parsing

  if (response == NULL) {
    g_print("Failed to parse server response.\n");
    return -1;
  }

  cJSON *action = cJSON_GetObjectItem(response, "action");
  if (!cJSON_IsString(action)) {
    g_print("Error: 'action' field is missing or not a string.\n");
    cJSON_Delete(response);
    return -1;
  }

  if (strcmp(action->valuestring, "LOGIN") == 0) {
    if (handle_login_response(response) == 0) {
      g_print("Login successful.\n");
      *logged_in = 1;
      gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "chats");

      // Send CHAT_LIST request to server
      cJSON *chat_list_request = cJSON_CreateObject();
      cJSON_AddStringToObject(chat_list_request, "action", "GET_CHAT_LIST");
      char *chat_list_request_str = cJSON_Print(chat_list_request);
      send(sock, chat_list_request_str, strlen(chat_list_request_str), 0);
      g_print("Sent: %s\n", chat_list_request_str);
      free(chat_list_request_str);
      cJSON_Delete(chat_list_request);
    } else {
      g_print("Error: Login Error.\n");
      gtk_label_set_text(GTK_LABEL(app_data->login_data->message),
                         "Wrong password or login");
    }
  } else if (strcmp(action->valuestring, "REGISTER") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Registration successful\n");
      gtk_style_context_add_class(
          gtk_widget_get_style_context(app_data->registration_data->message),
          "form-message-success");
      gtk_label_set_text(GTK_LABEL(app_data->registration_data->message),
                         "Registration was successful. Please log in!");
    } else {
      g_print("Error: Registration error.\n");
      gtk_label_set_text(GTK_LABEL(app_data->registration_data->message),
                         "Choose another username");
    }

  } else if (strcmp(action->valuestring, "FIND_USER") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      char username[50];
      strcpy(username, cJSON_GetObjectItem(response, "username")->valuestring);
      app_data->main_page->group_users[app_data->main_page->group_users_count]
          .button = gtk_button_new_with_label(username);
      strcpy(app_data->main_page
                 ->group_users[app_data->main_page->group_users_count]
                 .username,
             username);
      gtk_flow_box_insert(
          GTK_FLOW_BOX(app_data->main_page->group_box),
          app_data->main_page
              ->group_users[app_data->main_page->group_users_count]
              .button,
          -1);
      gtk_style_context_add_class(
          gtk_widget_get_style_context(
              app_data->main_page
                  ->group_users[app_data->main_page->group_users_count]
                  .button),
          "newchats-users-button");
      g_signal_connect(app_data->main_page
                           ->group_users[app_data->main_page->group_users_count]
                           .button,
                       "clicked", G_CALLBACK(removing_user),
                       app_data->main_page);
      gtk_widget_set_visible(
          app_data->main_page
              ->group_users[app_data->main_page->group_users_count]
              .button,
          1);
      app_data->main_page->group_users_count += 1;

    } else {
      gtk_label_set_text(
          GTK_LABEL(app_data->main_page->create_group_data.message),
          "User is not found");
    }
  } else if (strcmp(action->valuestring, "CREATE_CHAT") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Created_chat successful\n");
      ensure_cache_directory();
      save_single_chat_to_encrypted_cache(cJSON_GetObjectItem(response, "chat"),
                                          "cache", app_data);
      gtk_stack_set_visible_child_name(
          GTK_STACK(app_data->main_page->menu_stack), "chats_list");
      app_data->main_page->menu_opened = -1;
      gtk_stack_set_visible_child_name(
          GTK_STACK(app_data->main_page->central_area_stack), "chat");
    } else {
      g_print("Error: No such user.\n");
      gtk_label_set_text(
          GTK_LABEL(app_data->main_page->create_chat_data.message),
          "User is not found");
    }
  } else if (strcmp(action->valuestring, "LOGOUT") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("LOGOUT successful\n");
      delete_cache_directory();
      remove_all_chat_buttons(app_data->main_page);
      gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "login");
      delete_session();
    } else {
      g_print("Error: LOGOUT error.\n");
    }
  } else if (strcmp(action->valuestring, "CHAT_LIST") == 0) {
    ensure_cache_directory();
    handle_chat_list_response(response, "cache");
    create_chat_buttons_from_encrypted_cache(app_data->main_page, "cache");
    create_msg_buttons_from_cache(app_data->main_page, "cache");

  } else if (strcmp(action->valuestring, "MESSAGE_FROM_CHAT") == 0) {
    process_message_and_store(cJSON_Print(response), app_data);
  } else if (strcmp(action->valuestring, "VOICE_FROM_CHAT") == 0) {
    process_voice_message_and_store(cJSON_Print(response), app_data);
  } else if (strcmp(action->valuestring, "GET_PROFILE_DATA") == 0) {
    handle_get_profile_response(response, app_data);
  } else if (strcmp(action->valuestring, "UPDATE_PROFILE_DATA") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      char username[64] = {0};
      char serial_number[64] = {0};
      get_serial_number(serial_number, sizeof(serial_number));
      cJSON *pass_change = cJSON_GetObjectItem(response, "username_change");
      if (strcmp(pass_change->valuestring, "TRUE") == 0) {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(app_data->login_data->message),
            "form-message-success");
        gtk_label_set_label(
            GTK_LABEL(app_data->main_page->profile_data.username), "");
        gtk_label_set_text(GTK_LABEL(app_data->login_data->message),
                           "Username changed. Please log in!");
        cJSON *logout = cJSON_CreateObject();
        cJSON_AddStringToObject(logout, "action", "LOGOUT");
        char *logout_str = cJSON_Print(logout);
        cJSON_Delete(logout);
        send(app_data->main_page->sock, logout_str, strlen(logout_str), 0);
        g_print("Sent: %s\n", logout_str);
        free(logout_str);
        cJSON_Delete(response);
        return 0;
      }
      cJSON *json = cJSON_CreateObject();
      cJSON_AddStringToObject(json, "action", "GET_PROFILE_DATA");
      char *json_str = cJSON_Print(json);
      cJSON_Delete(json);
      send(app_data->main_page->sock, json_str, strlen(json_str), 0);
      g_print("Sent: %s\n", json_str);
      free(json_str);
    }
  } else if (strcmp(action->valuestring, "SEND_MESSAGE_TO_SERVER_STATUS") ==
             0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Sending  successful\n");
      process_message_and_store(cJSON_Print(response), app_data);
    } else {
      g_print("Error: Sending error.\n");
    }
  } else if (strcmp(action->valuestring,
                    "SEND_VOICE_MESSAGE_TO_SERVER_STATUS") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Sending  successful\n");
      process_voice_message_and_store(cJSON_Print(response), app_data);
    } else {
      g_print("Error: Sending error.\n");
    }
  }
  // Clean up JSON object
  cJSON_Delete(response);
  return 0;
}
