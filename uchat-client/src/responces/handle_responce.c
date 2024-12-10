#include <uchat.h>

#define INITIAL_BUFFER_SIZE                                                    \
  4096 // Adjust as necessary for expected response size

char *receive_large_json(int socket_fd) {
  size_t buffer_size = INITIAL_BUFFER_SIZE;
  char *buffer = g_malloc(buffer_size);
  if (!buffer) {
    perror("g_malloc failed");
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
      g_free(buffer);
      return (char *)-1; // Return -1 to indicate error
    } else if (activity == 0) {
      // Timeout occurred
      fprintf(stderr, "recv timed out.\n");
      g_free(buffer);
      return (char *)-1; // Return -1 for timeout
    }

    // If data is ready to be read
    bytes = recv(socket_fd, buffer + received, buffer_size - received, 0);
    if (bytes < 0) {
      perror("recv failed");
      g_free(buffer);
      return (char *)-1; // Return -1 for recv error
    } else if (bytes == 0) {
      // Connection closed by the server
      fprintf(stderr, "Server disconnected.\n");
      g_free(buffer);
      return (char *)-1; // Return -1 for disconnection
    }

    received += bytes;

    // If buffer is full, resize it
    if (received >= buffer_size) {
      buffer_size *= 2; // Double the buffer size
      char *temp = realloc(buffer, buffer_size);
      if (!temp) {
        perror("realloc failed");
        g_free(buffer);
        return (char *)-1; // Return -1 for memory allocation failure
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

int handle_response(int sock, int *logged_in, AppData *app_data) {
  char *buffer = receive_large_json(sock);
  if (buffer == (char *)-1) {
    g_print("Server disconnected or an error occurred.\n");
    *logged_in = 0;
    return -1; // Return -1 for disconnection or error
  }

  g_print("Buffer: %s\n", buffer);

  // Split and process each JSON object
  size_t len = strlen(buffer);
  int open_braces = 0, close_braces = 0;
  size_t start = 0;

  for (size_t i = 0; i < len; i++) {
    if (buffer[i] == '{') {
      open_braces++;
    } else if (buffer[i] == '}') {
      close_braces++;
    }

    // Check if we've found a complete JSON object
    if (open_braces > 0 && open_braces == close_braces) {
      size_t json_len = i - start + 1;
      char *json_str = g_malloc(json_len + 1);
      strncpy(json_str, buffer + start, json_len);
      json_str[json_len] = '\0';

      // Parse and process the individual JSON object
      cJSON *response = cJSON_Parse(json_str);
      if (!response) {
        g_print("Failed to parse JSON: %s\n", json_str);
        g_free(json_str);
        continue;
      }

      // Process the parsed JSON
      if (process_individual_response(response, logged_in, app_data) != 0) {
        return -1;
      }

      // Clean up
      cJSON_Delete(response);
      g_free(json_str);

      // Reset counters for the next JSON object
      open_braces = 0;
      close_braces = 0;
      start = i + 1;
    }
  }

  g_free(buffer); // Free the main buffer after processing
  return 0;
}

// Function to read and handle server responses
int process_individual_response(cJSON *response, int *logged_in,
                                AppData *app_data) {

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

      // Sync chat list after login
      sync_chat_list_with_server(sock, "cache");
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
      handle_logout(app_data);
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
    send_json(app_data->main_page->sock, "GET_PROFILE_DATA");
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
        if (logout_str) {
          g_free(logout_str);
          logout_str = NULL;
        }
        // Caused seg fault
        // cJSON_Delete(response);
        return 0;
      }
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
  } else if (strcmp(action->valuestring, "CALL") == 0) {
    process_voice_call_start(response, app_data);
  } else if (strcmp(action->valuestring, "ACCEPT_CALL") == 0) {
    process_voice_call_accept(response, app_data);
  } else if (strcmp(action->valuestring, "CALL_OFFLINE") == 0) {
    g_print("User is offline\n");
    gtk_label_set_text(GTK_LABEL(app_data->main_page->voice_call_window_label),
                       "Offline");
  } else if (strcmp(action->valuestring, "STOP_CALL") == 0) {
    process_voice_call_stop(response, app_data);
  } else if (strcmp(action->valuestring, "GET_NEW_DATA") == 0) {
    const char *json_str = cJSON_Print(response);
    handle_new_data_response(json_str, CACHE_DIR);
    create_chat_buttons_from_encrypted_cache(app_data->main_page, "cache");
    create_msg_buttons_from_cache(app_data->main_page, "cache");
  } else if (strcmp(action->valuestring, "UPDATE_MESSAGE_STATUS") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Updating  successful\n");
      process_message_update(cJSON_Print(response), app_data);
    } else {
      g_print("Error: Sending error.\n");
    }
  } else if (strcmp(action->valuestring, "UPDATE_MESSAGE_FROM_CHAT") == 0) {
    process_message_update_from_chat(cJSON_Print(response), app_data);
  } else if (strcmp(action->valuestring, "DELETE_MESSAGE_STATUS") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Updating  successful\n");
      process_message_delete(cJSON_Print(response), app_data);
    } else {
      g_print("Error: Sending error.\n");
    }
  } else if (strcmp(action->valuestring, "DELETE_MESSAGE_FROM_CHAT") == 0) {
    process_message_delete(cJSON_Print(response), app_data);
  } else if (strcmp(action->valuestring, "UPDATE_PASSWORD") == 0) {
    handle_up_pw_response(response, app_data);
  } else if (strcmp(action->valuestring, "GET_SETTINGS") == 0) {
    handle_get_settings_response(response, app_data);
  } else if (strcmp(action->valuestring,
                    "SEND_FILE_MESSAGE_TO_SERVER_STATUS") == 0) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Sending  successful\n");
      process_file_message_and_store(cJSON_Print(response), app_data);
    } else {
      g_print("Error: Sending error.\n");
    }
  } else if (strcmp(action->valuestring, "FILE_FROM_CHAT") == 0) {
    process_file_message_and_store(cJSON_Print(response), app_data);
  } else if (strcmp(action->valuestring, "CHAT_PROFILE_DATA") == 0) {
    process_chat_profile_data(cJSON_Print(response), app_data);
  }
  // Clean up JSON object
  // cJSON_Delete(response);
  return 0;
}
