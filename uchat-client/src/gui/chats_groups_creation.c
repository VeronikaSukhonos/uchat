#include <uchat.h>

void create_chat_buttons_from_encrypted_cache(t_main_page_data *main_page,
                                              const char *cache_dir) {
  DIR *dir = opendir(cache_dir);
  if (!dir) {
    perror("Failed to open cache directory");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG && strstr(entry->d_name, ".json")) {
      char file_path[256];
      snprintf(file_path, sizeof(file_path), "%s/%s", cache_dir, entry->d_name);

      char name[64] = {0}, chat_type[20] = {0}, last_message[1024] = {0};
      char last_sender[64] = {0}, last_time[32] = {0}, unread[16] = {0};
      int chat_id;

      if (read_chat_data_from_encrypted_json(
              file_path, &chat_id, name, chat_type, last_message, last_sender,
              last_time, unread) == 0) {

        // Pass all data to the button creation function
        new_chat_button_from_json(main_page, chat_id, name, chat_type,
                                  last_message, last_sender, last_time, unread);
      }
    }
  }
  closedir(dir);
}

void create_or_update_chat_button(t_main_page_data *main_page, int chat_id,
                                  const char *name, const char *chat_type,
                                  const char *last_message,
                                  const char *last_sender,
                                  const char *last_time, const char *unread) {
  // Debug: Print all input variables
  g_print("Updating/Creating Chat Button:\n");
  g_print("  chat_id: %d\n", chat_id);
  g_print("  name: %s\n", name);
  g_print("  chat_type: %s\n", chat_type);
  g_print("  last_message: %s\n", last_message);
  g_print("  last_sender: %s\n", last_sender);
  g_print("  last_time: %s\n", last_time);
  g_print("  unread: %s\n", unread);

  // Search for the chat in the linked list
  t_chat_node *current = main_page->chats;
  while (current) {
    if (current->chat.id == chat_id) {
      // Debug: Found existing chat
      g_print("Chat ID %d found, updating details...\n", chat_id);

      // Update existing chat button details
      if (current->chat.last_time) {
        gtk_label_set_text(GTK_LABEL(current->chat.last_time), last_time);
      } else {
        g_warning("last_time label is NULL for chat_id: %d", chat_id);
      }

      if (current->chat.last_sender) {
        gtk_label_set_text(GTK_LABEL(current->chat.last_sender), last_sender);
      } else {
        g_warning("last_sender label is NULL for chat_id: %d", chat_id);
      }

      if (current->chat.last_message) {
        gtk_label_set_text(GTK_LABEL(current->chat.last_message), last_message);
      } else {
        g_warning("last_message label is NULL for chat_id: %d", chat_id);
      }

      if (current->chat.unread) {
        gtk_label_set_text(GTK_LABEL(current->chat.unread), unread);
      } else {
        g_warning("unread label is NULL for chat_id: %d", chat_id);
      }

      // Highlight unread chats
      GtkStyleContext *style_context =
          gtk_widget_get_style_context(current->chat.button);
      if (style_context) {
        if (strlen(unread) > 0) {
          gtk_style_context_add_class(style_context,
                                      "chat-button-unread-border");
          g_print("Added 'unread-border' style for chat_id: %d\n", chat_id);
        } else {
          gtk_style_context_remove_class(style_context,
                                         "chat-button-unread-border");
          g_print("Removed 'unread-border' style for chat_id: %d\n", chat_id);
        }
      } else {
        g_warning("Style context is NULL for chat_id: %d", chat_id);
      }

      return; // Exit after updating the button
    }
    current = current->next;
  }

  // If chat_id does not exist, create a new chat button
  g_print("Chat ID %d not found, creating a new button...\n", chat_id);
  new_chat_button_from_json(main_page, chat_id, name, chat_type, last_message,
                            last_sender, last_time, unread);
}

gboolean is_user_scrolling = FALSE;

void on_scroll_changed(GtkAdjustment *chat_vadjustment, gpointer data) {
  double a_value = gtk_adjustment_get_value(chat_vadjustment);
  double a_upper = gtk_adjustment_get_upper(chat_vadjustment);
  double a_page_size = gtk_adjustment_get_page_size(chat_vadjustment);

  // If the user scrolls up, set the flag to TRUE
  if (a_value + a_page_size < a_upper - 1.0) {
    is_user_scrolling = TRUE;
  } else {
    // Re-enable auto-scroll when the user scrolls to the bottom
    is_user_scrolling = FALSE;
  }
}

void scroll_down(GtkAdjustment *chat_vadjustment, gpointer data) {
  // Only scroll down if the user is not manually scrolling
  if (!is_user_scrolling) {
    double a_upper = gtk_adjustment_get_upper(chat_vadjustment);
    double a_page_size = gtk_adjustment_get_page_size(chat_vadjustment);
    gtk_adjustment_set_value(chat_vadjustment, a_upper - a_page_size);
  }
}

void new_chat_button_from_json(t_main_page_data *main_page, int chat_id,
                               const char *name, const char *chat_type,
                               const char *last_message,
                               const char *last_sender, const char *last_time,
                               const char *unread) {
  t_chat_node *temp_node;
  if ((*main_page).chats == NULL) {
    (*main_page).chats = g_malloc(sizeof(t_chat_node));
    temp_node = (*main_page).chats;
  } else {
    temp_node = (*main_page).chats;
    while (temp_node->next != NULL)
      temp_node = temp_node->next;
    temp_node->next = g_malloc(sizeof(t_chat_node));
    temp_node = temp_node->next;
  }
  temp_node->next = NULL;

  (*temp_node).chat.id = chat_id; // Assign the chat_id

  // Create chat button
  (*temp_node).chat.button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX((*main_page).chats_box), (*temp_node).chat.button,
                     FALSE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.button), "menu-button");
  // Store chat type in the widget name
  gtk_widget_set_name((*temp_node).chat.button, chat_type);
  g_signal_connect((*temp_node).chat.button, "clicked", G_CALLBACK(show_chat),
                   main_page);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER((*temp_node).chat.button), main_box);

  // Set chat icon based on type
  g_print("Type:%s\n", chat_type);
  GtkWidget *image = gtk_image_new_from_file(
      strcmp(chat_type, "private") == 0
          ? "uchat-client/src/gui/resources/rabbit_chats.png"
          : "uchat-client/src/gui/resources/rabbits_chats.png");
  gtk_box_pack_start(GTK_BOX(main_box), image, FALSE, FALSE, 0);

  GtkWidget *text_boxes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(main_box), text_boxes, TRUE, TRUE, 0);

  // Top box for chat name and last message time
  GtkWidget *top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(text_boxes), top_box, TRUE, TRUE, 0);

  (*temp_node).chat.name = gtk_label_new(name);
  gtk_box_pack_start(GTK_BOX(top_box), (*temp_node).chat.name, TRUE, TRUE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.name),
      "chat-button-label");
  gtk_widget_set_halign((*temp_node).chat.name, GTK_ALIGN_START);

  (*temp_node).chat.last_time = gtk_label_new(last_time);
  gtk_box_pack_start(GTK_BOX(top_box), (*temp_node).chat.last_time, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.last_time),
      "chat-button-light");
  gtk_widget_set_halign((*temp_node).chat.last_time, GTK_ALIGN_END);

  // Bottom box for last sender, message, and unread count
  GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(text_boxes), bottom_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(bottom_box),
                              "chat-button-bottom-box");

  if (strlen(last_sender) > 0) {
    (*temp_node).chat.last_sender = gtk_label_new(last_sender);
    gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.last_sender,
                       FALSE, FALSE, 0);
    gtk_style_context_add_class(
        gtk_widget_get_style_context((*temp_node).chat.last_sender),
        "chat-button-sender");
    gtk_widget_set_halign((*temp_node).chat.last_sender, GTK_ALIGN_START);
  } else {
    (*temp_node).chat.last_sender = gtk_label_new("");
  }

  (*temp_node).chat.last_message = gtk_label_new(last_message);
  gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.last_message, TRUE,
                     TRUE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.last_message),
      "chat-button-light");
  gtk_widget_set_halign((*temp_node).chat.last_message, GTK_ALIGN_START);

  (*temp_node).chat.unread = gtk_label_new(unread);
  gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.unread, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.unread),
      "chat-button-unread");
  gtk_widget_set_halign((*temp_node).chat.unread, GTK_ALIGN_END);

  GtkWidget *dialog_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_style_context_add_class(gtk_widget_get_style_context(dialog_scroll),
                              "dialog-scroll");
  GtkTargetEntry targets[] = {{"text/uri-list", GTK_TARGET_OTHER_APP, 0}};
  gtk_drag_dest_set(dialog_scroll, GTK_DEST_DEFAULT_ALL, targets, 1,
                    GDK_ACTION_COPY);
  g_signal_connect(dialog_scroll, "drag-data-received",
                   G_CALLBACK(on_drag_data_received), main_page);

  GtkAdjustment *chat_vadjustment =
      gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(dialog_scroll));

  // Monitor user scrolling
  g_signal_connect(chat_vadjustment, "value-changed",
                   G_CALLBACK(on_scroll_changed), NULL);

  // Handle new messages (scroll down only if the user is not manually
  // scrolling)
  g_signal_connect(chat_vadjustment, "changed", G_CALLBACK(scroll_down), NULL);

  // Convert chat_id to string and set it as the child name
  char id_str[32];
  snprintf(id_str, sizeof(id_str), "%d", chat_id);
  gtk_stack_add_named(GTK_STACK((*main_page).chats_stack), dialog_scroll,
                      id_str);

  (*temp_node).chat.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(dialog_scroll), (*temp_node).chat.box);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*temp_node).chat.box), "chat-box");

  (*temp_node).chat.changing_message = NULL;

  // Highlight unread chats
  if (strlen(unread) > 0)
    gtk_style_context_add_class(
        gtk_widget_get_style_context((*temp_node).chat.button),
        "chat-button-unread-border");

  // Make all widgets visible
  gtk_widget_set_visible((*temp_node).chat.button, 1);
  gtk_widget_set_visible(image, 1);
  gtk_widget_set_visible(main_box, 1);
  gtk_widget_set_visible(text_boxes, 1);
  gtk_widget_set_visible(top_box, 1);
  gtk_widget_set_visible(bottom_box, 1);
  gtk_widget_set_visible((*temp_node).chat.name, 1);
  gtk_widget_set_visible((*temp_node).chat.last_time, 1);
  if ((*temp_node).chat.last_sender != NULL)
    gtk_widget_set_visible((*temp_node).chat.last_sender, 1);
  gtk_widget_set_visible((*temp_node).chat.last_message, 1);
  gtk_widget_set_visible((*temp_node).chat.unread, 1);
  gtk_widget_set_visible(dialog_scroll, 1);
  gtk_widget_set_visible((*temp_node).chat.box, 1);

  (*main_page).chats_count += 1;
}

void remove_all_chat_buttons(t_main_page_data *main_page) {
  t_chat_node *current = main_page->chats;
  t_chat_node *next_node;

  while (current != NULL) {
    next_node = current->next;

    // Destroy GTK widgets
    if (current->chat.name) {
      gtk_widget_destroy(current->chat.name);
    }
    if (current->chat.last_time) {
      gtk_widget_destroy(current->chat.last_time);
    }
    if (current->chat.last_sender) {
      gtk_widget_destroy(current->chat.last_sender);
    }
    if (current->chat.last_message) {
      gtk_widget_destroy(current->chat.last_message);
    }
    if (current->chat.unread) {
      gtk_widget_destroy(current->chat.unread);
    }
    if (current->chat.box) {
      gtk_widget_destroy(current->chat.box);
    }
    if (current->chat.button) {
      gtk_widget_destroy(current->chat.button);
    }

    // Free the current node
    g_free(current);
    current = next_node;
  }

  // Reset the chats linked list
  main_page->chats = NULL;
  main_page->chats_count = 0;

  // Clear the GTK stack (if necessary)
  GList *children =
      gtk_container_get_children(GTK_CONTAINER(main_page->chats_stack));
  for (GList *child = children; child != NULL; child = child->next) {
    gtk_widget_destroy(GTK_WIDGET(child->data));
  }
  g_list_free(children);
}

void chat_creation(GtkWidget *create_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_chat_data.username));

  if (strcmp(username, "") == 0) {
    gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message),
                       "Username is required");
  } else {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "CREATE_CHAT");
    cJSON_AddStringToObject(json, "username", username);
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);
    send(main_page->sock, json_str, strlen(json_str), 0);
    g_print("Sent: %s\n", json_str);
    g_free(json_str);
    // if (получилось) {
    // new_chat_button(main_page, username, 'c');
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_chat_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message), "");
    // }
  }
}

void removing_user(GtkWidget *clicked_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  for (int i = 0; i < (*main_page).group_users_count; ++i) {
    if ((*main_page).group_users[i].button == clicked_button) {
      gtk_widget_destroy(gtk_widget_get_parent(clicked_button));
      (*main_page).group_users_count -= 1;
      int j;
      for (j = i; j < (*main_page).group_users_count; ++j) {
        (*main_page).group_users[j].button =
            (*main_page).group_users[j + 1].button;
        strcpy((*main_page).group_users[j].username,
               (*main_page).group_users[j + 1].username);
      }
      (*main_page).group_users[j].button = NULL;
      // strcpy((*main_page).group_users[i].username, "");
    }
  }
  gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
}

void adding_user(GtkWidget *add_user_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_group_data.username));

  if (check_form_data(username, NULL, (*main_page).create_group_data.message) ==
      1) {
    if ((*main_page).group_users_count == USERS_IN_GROUP_COUNT) {
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                         "Maximum number of members is reached");
    } else {
      int already_in_group = 0;
      for (int i = 0; i < (*main_page).group_users_count; ++i) {
        if (strcmp(username, (*main_page).group_users[i].username) == 0) {
          already_in_group = 1;
          break;
        }
      }
      if (already_in_group == 0) {
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "action", "FIND_USER");
        cJSON_AddStringToObject(json, "username", username);
        char *json_str = cJSON_Print(json);
        cJSON_Delete(json);
        send(main_page->sock, json_str, strlen(json_str), 0);
        g_print("Sent: %s\n", json_str);
        g_free(json_str);
      }

      gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username),
                         "");
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
    }
  }
}

void group_creation(GtkWidget *create_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *name = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_group_data.name));

  if (strcmp(name, "") == 0) {
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have a name");
  } else if (strlen(name) > 40) {
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group name cannot exceed 40 characters");
  }
  /*else if ((*main_page).group_users_count < 2)
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have at least three members");*/
  else {
    g_print("Group %s with users ", name);
    char usernames[(*main_page).group_users_count][50];
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      g_print("%s%c", (*main_page).group_users[i].username,
              i == (*main_page).group_users_count - 1 ? '\n' : ' ');
      strcpy(usernames[i], (*main_page).group_users[i].username);
    }
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      gtk_widget_destroy(
          gtk_widget_get_parent((*main_page).group_users[i].button));
      strcpy((*main_page).group_users[i].username, "");
    }
    char *json_str =
        build_json_group_chat(name, usernames, (*main_page).group_users_count);
    send((*main_page).sock, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    g_free(json_str);
    (*main_page).group_users_count = 0;
    //    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
    //                                   "chats_list");
    //(*main_page).menu_opened = -1;
    // gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
    //                               "chat");
    // new_chat_button(main_page, name, 'g');
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
  }
}
