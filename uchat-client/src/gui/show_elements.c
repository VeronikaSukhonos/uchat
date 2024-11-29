#include <uchat.h>

void remove_buttons(t_main_page_data *main_page) {
  for (t_chat_node *i = main_page->chats; i != NULL; i = i->next) {
    if (main_page->opened_chat == &i->chat) {
      GtkWidget *chat_box =
          i->chat.box; // The container for the message buttons

      // Ensure chat_box is valid
      if (!chat_box) {
        g_print("Error: chat_box is NULL for chat ID: %d\n", i->chat.id);
        continue;
      }

      // Remove all message buttons from the chat's container (box)
      for (MessageNode *msg_node = main_page->messages; msg_node != NULL;
           msg_node = msg_node->next) {
        if (msg_node->message->chat_id == i->chat.id) {
          if (msg_node->message->button != NULL) {
            gtk_container_remove(GTK_CONTAINER(chat_box),
                                 msg_node->message->button);
            g_print("Removed button for message ID: %d\n",
                    msg_node->message->message_id);
          } else {
            g_print("Warning: button is NULL for message ID: %d\n",
                    msg_node->message->message_id);
          }
        }
      }
    }
  }
}

void show_new_chat(GtkWidget *new_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  remove_buttons(main_page);
  set_selected_button(&(*main_page).menu_button_selected, &new_chat_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_chat");
  (*main_page).opened_chat = NULL;
}

void show_new_group(GtkWidget *new_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  remove_buttons(main_page);
  set_selected_button(&(*main_page).menu_button_selected, &new_group_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_group");
  (*main_page).opened_chat = NULL;
}

void show_profile(GtkWidget *settings_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  remove_buttons(main_page);
  set_selected_button(&(*main_page).menu_button_selected, &settings_button);

  // Set default or current username and description
  gtk_label_set_label(
      GTK_LABEL((*main_page).profile_data.username),
      strcmp((char *)gtk_label_get_label(
                 GTK_LABEL((*main_page).profile_data.username)),
             "") == 0
          ? ""
          : gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.status), "online");

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "GET_PROFILE_DATA");
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);

  g_free(json_str);
  // Set the full description with default name and group
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.description),
                      strcmp((char *)gtk_label_get_label(GTK_LABEL(
                                 (*main_page).profile_data.description)),
                             "") == 0
                          ? "Connecting..."
                          : gtk_label_get_label(GTK_LABEL(
                                (*main_page).profile_data.description)));

  // Set visible
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "user_info");
  (*main_page).opened_chat = NULL;
}

void show_edit_page(GtkWidget *edit_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  remove_buttons(main_page);

  // Set existing values in the edit fields
  gtk_entry_set_text(
      GTK_ENTRY((*main_page).edit_data.username),
      gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
  gtk_entry_set_text(
      GTK_ENTRY((*main_page).edit_data.name_surname),
      gtk_label_get_label(GTK_LABEL((*main_page).profile_data.name_surname)));
  gtk_entry_set_text(
      GTK_ENTRY((*main_page).edit_data.student_group),
      gtk_label_get_label(GTK_LABEL((*main_page).profile_data.student_group)));

  // Set placeholder text
  gtk_entry_set_placeholder_text(GTK_ENTRY((*main_page).edit_data.username),
                                 "Username");
  gtk_entry_set_placeholder_text(GTK_ENTRY((*main_page).edit_data.name_surname),
                                 "Name Surname");
  gtk_entry_set_placeholder_text(
      GTK_ENTRY((*main_page).edit_data.student_group), "Student Group");

  gtk_label_set_label(GTK_LABEL((*main_page).edit_data.message), "");

  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "edit_profile");
}

void show_chat(GtkWidget *chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Set the selected button for the menu
  set_selected_button(&main_page->menu_button_selected, &chat_button);

  // Show the chat stack and hide the chats list
  gtk_stack_set_visible_child_name(GTK_STACK(main_page->central_area_stack),
                                   "chat");
  gtk_stack_set_visible_child_name(GTK_STACK(main_page->menu_stack),
                                   "chats_list");

  // Check if any chat has the microphone active, stop it and update the
  // button image
  for (t_chat_node *i = main_page->chats; i != NULL; i = i->next) {
    if (i->chat.is_mic_active) {
      // Stop recording if any chat's mic is active
      i->chat.is_mic_active = FALSE;

      // Update the image on the mic button to indicate the recording has
      // stopped
      GtkWidget *mic_button_img_start = gtk_image_new_from_file(
          "uchat-client/src/gui/resources/voice-start.png");
      gtk_button_set_image(GTK_BUTTON(main_page->mic_button),
                           mic_button_img_start);

      // If the opened chat is the current chat, also stop its mic recording
      if (main_page->opened_chat == &i->chat) {
        main_page->opened_chat->is_mic_active = FALSE;
      }

      // You can add a log or other actions as needed
      stop_recording();
      g_print("Recording stopped for chat: %d\n", i->chat.id);
    }
  }

  // Delete existing message buttons for the current chat
  for (t_chat_node *i = main_page->chats; i != NULL; i = i->next) {
    if (main_page->opened_chat == &i->chat) {
      GtkWidget *chat_box =
          i->chat.box; // The container for the message buttons

      // Ensure chat_box is valid
      if (!chat_box) {
        g_print("Error: chat_box is NULL for chat ID: %d\n", i->chat.id);
        continue;
      }

      // Remove all message buttons from the chat's container (box)
      for (MessageNode *msg_node = main_page->messages; msg_node != NULL;
           msg_node = msg_node->next) {
        if (msg_node->message->chat_id == i->chat.id) {
          if (msg_node->message->button != NULL) {
            gtk_container_remove(GTK_CONTAINER(chat_box),
                                 msg_node->message->button);
            g_print("Removed button for message ID: %d\n",
                    msg_node->message->message_id);
          } else {
            g_print("Warning: button is NULL for message ID: %d\n",
                    msg_node->message->message_id);
          }
        }
      }
    }
  }

  // Iterate through all chats and set the opened chat
  for (t_chat_node *i = main_page->chats; i != NULL; i = i->next) {
    if (i->chat.button == chat_button) {
      main_page->opened_chat = &i->chat;
      char id_str[10];
      snprintf(id_str, sizeof(id_str), "%d", i->chat.id);
      gtk_stack_set_visible_child_name(GTK_STACK(main_page->chats_stack),
                                       id_str);

      // Get chat data from cache to find the other member's username
      // Get chat data from cache to find the other member's username or group
      // name
      char file_path[256];
      snprintf(file_path, sizeof(file_path), "cache/chat_%d.json", i->chat.id);
      char *decrypted_json = decrypt_json_from_file(file_path);

      if (decrypted_json) {
        cJSON *json = cJSON_Parse(decrypted_json);
        g_free(decrypted_json);

        if (json) {
          // Check if it's a group chat or private chat
          cJSON *chat_type_json = cJSON_GetObjectItem(json, "type");
          if (chat_type_json && cJSON_IsString(chat_type_json)) {
            const char *chat_type = chat_type_json->valuestring;

            // If it's a group chat, set the nickname to the group name
            if (strcmp(chat_type, "private") != 0) {
              cJSON *group_name_json = cJSON_GetObjectItem(json, "name");
              if (group_name_json && cJSON_IsString(group_name_json)) {
                if (main_page->chat_nickname) {
                  gtk_label_set_text(GTK_LABEL(main_page->chat_nickname),
                                     group_name_json->valuestring);
                }
              }
            } else {
              // Otherwise, it's a private chat, find the other member's
              // username
              cJSON *members = cJSON_GetObjectItem(json, "members");
              if (cJSON_IsArray(members)) {
                cJSON *member;
                cJSON_ArrayForEach(member, members) {
                  cJSON *username_json =
                      cJSON_GetObjectItem(member, "username");
                  if (username_json &&
                      strcmp(username_json->valuestring, username) != 0) {
                    // Found the other member's username
                    if (main_page->chat_nickname) {
                      gtk_label_set_text(GTK_LABEL(main_page->chat_nickname),
                                         username_json->valuestring);
                    }
                    break;
                  }
                }
              }
            }
          }
          cJSON_Delete(json);
        }
      }

      // Create new message buttons for this chat
      for (MessageNode *msg_node = main_page->messages; msg_node != NULL;
           msg_node = msg_node->next) {
        if (msg_node == NULL) {
          g_print("msg_node in NULL\n");
          break;
        }
        if (msg_node->message == NULL) {
          g_print("msg_node in NULL\n");
          break;
        }
        if (msg_node->message->chat_id == i->chat.id) {
          create_message_button(
              main_page, msg_node); // Create a new button for each message
          g_print("Created button for message ID: %d\n",
                  msg_node->message->message_id);
        }
      }

      break; // Exit loop after setting the opened chat
    }
  }
}

void show_profile_from_icon(GtkWidget *profile_icon, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Remove any selected buttons from the menu
  remove_buttons(main_page);

  // Request profile data from server
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "GET_PROFILE_DATA");
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);
  g_free(json_str);

  // Set the visible child to user_info with a smooth transition
  gtk_stack_set_transition_type(GTK_STACK((*main_page).central_area_stack),
                                GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "user_info");
  (*main_page).opened_chat = NULL;
}

void show_login(GtkWidget *login_link_button, t_form_data *data) {
  GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));

  gtk_stack_set_visible_child_name(GTK_STACK(pages), "login");
  /*gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_parent(pages)), NULL);*/
  gtk_entry_set_text(GTK_ENTRY(data->username), "");
  gtk_entry_set_text(GTK_ENTRY(data->password), "");
  gtk_entry_set_text(GTK_ENTRY(data->repassword), "");
  gtk_label_set_text(GTK_LABEL(data->message), "");
  gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
  gtk_entry_set_visibility(GTK_ENTRY(data->repassword), TRUE);
  change_password_visibility(data->pw_button, data->password);
  change_password_visibility(data->repw_button, data->repassword);
}

void show_registration(GtkWidget *registration_link_button, t_form_data *data) {
  GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));

  gtk_stack_set_visible_child_name(GTK_STACK(pages), "registration");
  /*gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_parent(pages)), NULL);*/
  gtk_entry_set_text(GTK_ENTRY(data->username), "");
  gtk_entry_set_text(GTK_ENTRY(data->password), "");
  gtk_label_set_text(GTK_LABEL(data->message), "");
  gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
  change_password_visibility(data->pw_button, data->password);
}

void show_participant_profile(GtkWidget *profile_icon, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Create a new dialog window
  GtkWidget *dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(dialog), "User Profile");
  gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

  // Create main container
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_container_add(GTK_CONTAINER(dialog), content_box);
  gtk_style_context_add_class(gtk_widget_get_style_context(content_box),
                              "profile-content");

  // Profile picture
  GtkWidget *avatar = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/rabbit_profile.png");
  gtk_box_pack_start(GTK_BOX(content_box), avatar, FALSE, FALSE, 0);
  gtk_widget_set_size_request(avatar, 150, 150);
  gtk_widget_set_halign(avatar, GTK_ALIGN_CENTER);

  // Username
  GtkWidget *username_label = gtk_label_new(NULL);
  if (main_page->chat_nickname) {
    gtk_label_set_text(GTK_LABEL(username_label),
                       gtk_label_get_text(GTK_LABEL(main_page->chat_nickname)));
  }
  gtk_box_pack_start(GTK_BOX(content_box), username_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(username_label),
                              "profile-username");

  // Status
  GtkWidget *status_label = gtk_label_new("online");
  gtk_box_pack_start(GTK_BOX(content_box), status_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(status_label),
                              "profile-status");

  // Info grid
  GtkWidget *info_grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(info_grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(info_grid), 15);
  gtk_box_pack_start(GTK_BOX(content_box), info_grid, TRUE, TRUE, 20);

  // Request user info from server
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "GET_USER_INFO");
  if (main_page->chat_nickname) {
    cJSON_AddStringToObject(
        json, "username",
        gtk_label_get_text(GTK_LABEL(main_page->chat_nickname)));
  }
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);
  g_free(json_str);

  // Show all widgets
  gtk_widget_show_all(dialog);

  // Connect destroy signal
  g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}
