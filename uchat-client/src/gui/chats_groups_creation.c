#include <uchat.h>

void new_chat_button(t_main_page_data *main_page, char *name, char chat_type) {
  t_chat_node *temp_node;
  if ((*main_page).chats == NULL) {
    (*main_page).chats = malloc(sizeof(t_chat_node));
    temp_node = (*main_page).chats;
  }
  else {
    temp_node = (*main_page).chats;
    while (temp_node->next != NULL)
      temp_node = temp_node->next;
    temp_node->next = malloc(sizeof(t_chat_node));
    temp_node = temp_node->next;
  }
  temp_node->next = NULL;

  int n = (*main_page).chats_count;
  int k = 0;
  (*temp_node).chat.id[k++] = n % 10 + '0';
  n /= 10;
  while  (n > 0) {
    (*temp_node).chat.id[k++] = n % 10 + '0';
    n /= 10;
  }
  (*temp_node).chat.id[k] = '\0';
  int j;
  for (k = 0, j = strlen((*temp_node).chat.id) - 1; k < j; ++k, --j) {
    char temp_char = (*temp_node).chat.id[k];
    (*temp_node).chat.id[k] = (*temp_node).chat.id[j];
    (*temp_node).chat.id[j] = temp_char;
  }

  (*temp_node).chat.button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX((*main_page).chats_box), (*temp_node).chat.button,
                     FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.button),
                            "menu-button");
  g_signal_connect((*temp_node).chat.button,
                   "clicked", G_CALLBACK(show_chat), main_page);

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER((*temp_node).chat.button), main_box);

  GtkWidget *image = gtk_image_new_from_file(chat_type == 'c'
                                             ? "uchat-client/src/gui/resources/rabbit_chats.png"
                                             : "uchat-client/src/gui/resources/rabbits_chats.png");
  gtk_box_pack_start(GTK_BOX(main_box), image, FALSE, FALSE, 0);

  GtkWidget *text_boxes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(main_box), text_boxes, TRUE, TRUE, 0);
  GtkWidget *top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(text_boxes), top_box, TRUE, TRUE, 0);
  GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(text_boxes), bottom_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(bottom_box),
                              "chat-button-bottom-box");

  (*temp_node).chat.name = gtk_label_new(name);
  gtk_box_pack_start(GTK_BOX(top_box), (*temp_node).chat.name,
                     TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.name),
                              "chat-button-label");
  gtk_widget_set_halign((*temp_node).chat.name, GTK_ALIGN_START);

  (*temp_node).chat.last_time = gtk_label_new("00:00");
  gtk_box_pack_start(GTK_BOX(top_box), (*temp_node).chat.last_time,
                     FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.last_time),
                              "chat-button-light");
  gtk_widget_set_halign((*temp_node).chat.last_time, GTK_ALIGN_END);

  int string_length = 30;
  char *last_sender = chat_type == 'c'
                      ? ""
                      : atoi((*temp_node).chat.id) % 2 == 0
                      ? "ifranko"
                      : "tshevchenko";
  char *last_message = atoi((*temp_node).chat.id) % 3 == 0
                       ? "Hello!"
                       : "Heeeyy!!! Can't wait!!!!!";
  char *unread = atoi((*temp_node).chat.id) % 3 != 0
                 ? atoi((*temp_node).chat.id) % 2 == 0
                 ? "10"
                 : "3"
                 : "";
  string_length -= strlen(last_sender) + strlen(unread);
  int last_message_length = strlen(last_message);
  char last_message_res[string_length + 1];
  for (int i = 0; i < string_length; ++i)
    last_message_res[i] = i >= last_message_length
                          ? '\0'
                          : last_message_length > string_length && i >= string_length - 3
                          ? '.'
                          : last_message[i];
  last_message_res[string_length] = '\0';

  if (strcmp(last_sender, "") != 0) {
    (*temp_node).chat.last_sender = gtk_label_new(last_sender);
    gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.last_sender,
                       FALSE, FALSE, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.last_sender),
                                "chat-button-sender");
    gtk_widget_set_halign((*temp_node).chat.last_sender, GTK_ALIGN_START);
  }
  else
    (*temp_node).chat.last_sender = NULL;

  (*temp_node).chat.last_message = gtk_label_new(last_message_res);
  gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.last_message,
                     TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.last_message),
                              "chat-button-light");
  gtk_widget_set_halign((*temp_node).chat.last_message, GTK_ALIGN_START);

  (*temp_node).chat.unread = gtk_label_new(unread);
  gtk_box_pack_start(GTK_BOX(bottom_box), (*temp_node).chat.unread,
                     FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.unread),
                              "chat-button-unread");
  gtk_widget_set_halign((*temp_node).chat.unread, GTK_ALIGN_END);

  (*temp_node).chat.box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).chats_stack), (*temp_node).chat.box,
                      (*temp_node).chat.id);

  if (strcmp(unread, "") != 0)
    gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).chat.button),
                            "chat-button-unread-border");

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
  gtk_widget_set_visible((*temp_node).chat.box, 1);
  (*main_page).chats_count += 1;
  show_chat((*temp_node).chat.button, main_page);
}

void chat_creation(GtkWidget *create_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_chat_data.username));

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "CREATE_CHAT");
  cJSON_AddStringToObject(json, "username", username);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);
  free(json_str);
  // if (получилось) {
  new_chat_button(main_page, username, 'c');
  gtk_entry_set_text(GTK_ENTRY((*main_page).create_chat_data.username), "");
  gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message), "");
  // }
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
      cJSON *json = cJSON_CreateObject();
      cJSON_AddStringToObject(json, "action", "FIND_USER");
      cJSON_AddStringToObject(json, "username", username);
      char *json_str = cJSON_Print(json);
      cJSON_Delete(json);
      send(main_page->sock, json_str, strlen(json_str), 0);
      g_print("Sent: %s\n", json_str);
      free(json_str);

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

  if (strcmp(name, "") == 0)
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have a name");
  else if ((*main_page).group_users_count < 2)
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have at least three members");
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
    free(json_str);
    (*main_page).group_users_count = 0;
    //    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
    //                                   "chats_list");
    //(*main_page).menu_opened = -1;
    // gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
    //                               "chat");
    new_chat_button(main_page, name, 'g');
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
  }
}

