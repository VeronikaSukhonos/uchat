#include <uchat.h>

void send_message_to_server(int chat_id, const gchar *message) {
  cJSON *json_message = cJSON_CreateObject();
  cJSON_AddStringToObject(json_message, "action", "SEND_MESSAGE_TO_CHAT");
  cJSON_AddNumberToObject(json_message, "chat_id", chat_id);
  cJSON_AddStringToObject(json_message, "message", message);

  char *json_string = cJSON_Print(json_message);
  g_print("Sending message to server: %s\n", json_string);

  cJSON_Delete(json_message);
  free(json_string);
}

void send_message_f(GtkWidget *widget, gpointer data) {
  GtkEntry *message_entry = GTK_ENTRY(data);
  const gchar *message_text = gtk_entry_get_text(message_entry);
  // after using t_chat_form_data->username appears segmentation fault

  int chat_id = -1; // need to be replaced this with current username

  if (g_strcmp0(message_text, "") != 0) {
    send_message_to_server(chat_id, message_text);
    gtk_entry_set_text(message_entry, "");
  } else {
    g_print("Cannot send an empty message.\n");
  }
}

void open_close_menu(GtkWidget *menu_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
                                   (*main_page).menu_opened == 1 ? "chats_list"
                                                                 : "menu");
  (*main_page).menu_opened *= -1;
}

void set_selected_button(GtkWidget **selected_button, GtkWidget **new_selected_button) {
  if (*selected_button != NULL)
    gtk_style_context_remove_class(gtk_widget_get_style_context(*selected_button),
                                "menu-button-selected");
  *selected_button = *new_selected_button;
  gtk_style_context_add_class(gtk_widget_get_style_context(*selected_button),
                              "menu-button-selected");
}

void show_new_chat(GtkWidget *new_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &new_chat_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_chat");
}

void show_new_group(GtkWidget *new_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &new_group_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_group");
}

void show_profile(GtkWidget *settings_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &settings_button);
  // username and description from database
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.username), strcmp((char *)gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)), "") == 0 ? "yrezchyk": gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.status), "online");
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.description), strcmp((char *)gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)), "") == 0 ? "Yevheniia Rezchyk\n@yrezchyk | KN-423k | Student" : gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)));
  // set visible
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "user_info");
}

// show edit page
void show_edit_page(GtkWidget *edit_button, gpointer data) {
    t_main_page_data *main_page = (t_main_page_data *)data;
    gtk_entry_set_text(GTK_ENTRY((*main_page).edit_data.username), gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
    gtk_entry_set_text(GTK_ENTRY((*main_page).edit_data.description), gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)));
    gtk_label_set_label(GTK_LABEL((*main_page).edit_data.message), "");
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack), "edit_profile");
}

// change profile
void change_profile(GtkWidget *change_button, gpointer data) {
    t_main_page_data *main_page = (t_main_page_data *)data;
    char *username = (char *)gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.username));
    if (check_form_data(username, NULL, (*main_page).edit_data.message) == 1) {
        gtk_label_set_label(GTK_LABEL((*main_page).profile_data.username), gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.username)));
        gtk_label_set_label(GTK_LABEL((*main_page).profile_data.description), gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.description)));
        gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack), "user_info");
    }
}

void log_out(GtkWidget *log_out_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "LOGOUT");
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);
  free(json_str);
  // gtk_stack_set_visible_child_name(GTK_STACK((*GtkWidget)data), "login");
}

void show_chat(GtkWidget *chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &chat_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "chat");
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
  gtk_entry_set_text(GTK_ENTRY((*main_page).create_chat_data.username), "");
  gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message), "");
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
    //gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
      //                               "chat");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
  }
}

void create_chats_page(GtkWidget *pages, GtkWidget *chats,
                       t_main_page_data *main_page) {
  GtkWidget *sidebar;
  GtkWidget *menu_box, *chats_list;
  GtkWidget *create_chat, *create_group, *user_info, *edit_profile, *chat;
  GtkWidget *img_profile;

  chats = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_stack_add_named(GTK_STACK(pages), chats, "chats");
  gtk_style_context_add_class(gtk_widget_get_style_context(chats), "box");

  // sidebar
  sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(chats), sidebar, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "menu");
  gtk_widget_set_size_request(GTK_WIDGET(sidebar), 270, -1);

  GtkWidget *sidebar_top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(sidebar), sidebar_top, FALSE, FALSE, 0);

  GtkWidget *menu_button = gtk_button_new_with_label("...");
  gtk_box_pack_start(GTK_BOX(sidebar_top),
                     menu_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_button),
                              "dots");
  g_signal_connect(menu_button, "clicked", G_CALLBACK(open_close_menu),
                   main_page);

  GtkWidget *menu_label = gtk_label_new("gChats");
  gtk_box_pack_start(GTK_BOX(sidebar_top),
                     menu_label, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label),
                              "gchats");
  gtk_widget_set_halign(menu_label, GTK_ALIGN_CENTER);

  // stack
  (*main_page).menu_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(sidebar), (*main_page).menu_stack, FALSE, FALSE, 0);
  (*main_page).menu_opened = 1;
  (*main_page).menu_button_selected = NULL;

  menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), menu_box, "menu");

  GtkWidget *new_chat_button = gtk_button_new();
  GtkWidget *new_chat_button_label = gtk_label_new("New chat");
  gtk_container_add(GTK_CONTAINER(new_chat_button), new_chat_button_label);
  gtk_widget_set_halign(new_chat_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(menu_box), new_chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_chat_button),
                              "menu-button");
  g_signal_connect(new_chat_button, "clicked", G_CALLBACK(show_new_chat),
                   main_page);

  GtkWidget *new_group_button = gtk_button_new();
  GtkWidget *new_group_button_label = gtk_label_new("New group");
  gtk_container_add(GTK_CONTAINER(new_group_button), new_group_button_label);
  gtk_widget_set_halign(new_group_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(menu_box), new_group_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_group_button),
                              "menu-button");
  g_signal_connect(new_group_button, "clicked", G_CALLBACK(show_new_group),
                   main_page);

  GtkWidget *profile_button = gtk_button_new();
  GtkWidget *profile_button_label = gtk_label_new("Profile");
  gtk_container_add(GTK_CONTAINER(profile_button), profile_button_label);
  gtk_widget_set_halign(profile_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(menu_box), profile_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(profile_button),
                              "menu-button");
  g_signal_connect(profile_button, "clicked", G_CALLBACK(show_profile),
                   main_page);

  GtkWidget *log_out_button = gtk_button_new();
  GtkWidget *log_out_button_label = gtk_label_new("Log out");
  gtk_container_add(GTK_CONTAINER(log_out_button), log_out_button_label);
  gtk_widget_set_halign(log_out_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(menu_box), log_out_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(log_out_button),
                              "menu-button");
  g_signal_connect(log_out_button, "clicked", G_CALLBACK(log_out), main_page);

  GtkWidget *chats_list_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), chats_list_box, "chats_list");

  GtkWidget *chat_button = gtk_button_new();
  GtkWidget *chat_button_label = gtk_label_new("Chat");
  gtk_container_add(GTK_CONTAINER(chat_button), chat_button_label);
  gtk_widget_set_halign(chat_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(chats_list_box), chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(chat_button),
                              "menu-button");
  g_signal_connect(chat_button, "clicked", G_CALLBACK(show_chat),
                   main_page);
  // central area
  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(chats), (*main_page).central_area_stack, TRUE,
                     TRUE, 0);

  GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), message_box,
                      "chat");

  (*main_page).chats_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(message_box), (*main_page).chats_stack,
                     TRUE, TRUE, 0);

  // stack for chat messages
  GtkWidget *chat_label = gtk_label_new("Chat messages will appear here");
  gtk_stack_add_named(GTK_STACK((*main_page).chats_stack), chat_label,
                      "chat");

  // input box for sending messages
  GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(input_box),
                              "input-box");
  gtk_box_pack_start(GTK_BOX(message_box), input_box, FALSE, FALSE, 5);

  GtkWidget *message_entry = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry),
                              "message-entry");
  gtk_box_pack_start(GTK_BOX(input_box), message_entry, TRUE, TRUE, 5);

  GtkWidget *send_button = gtk_button_new_with_label("Send");
  gtk_style_context_add_class(gtk_widget_get_style_context(send_button),
                              "send-button");
  gtk_box_pack_start(GTK_BOX(input_box), send_button, FALSE, FALSE, 5);
  g_signal_connect(send_button, "clicked", G_CALLBACK(send_message_f),
                   message_entry);

  // create chat
  create_chat = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), create_chat,
                      "create_chat");

  (*main_page).create_chat_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(create_chat), (*main_page).create_chat_data.form,
                     TRUE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.form),
      "form");
  gtk_widget_set_halign((*main_page).create_chat_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).create_chat_data.form), 450, -1);

  GtkWidget *create_chat_label = gtk_label_new("New chat");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(create_chat_label), "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).create_chat_data.form),
                     create_chat_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(create_chat_label, GTK_ALIGN_CENTER);

  GtkWidget *create_chat_username_label = gtk_label_new("Username");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(create_chat_username_label), "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).create_chat_data.form),
                     create_chat_username_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(create_chat_username_label, GTK_ALIGN_START);

  GtkWidget *chat_entry_and_button = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX((*main_page).create_chat_data.form),
                     chat_entry_and_button, FALSE, FALSE, 0);

  (*main_page).create_chat_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.username),
      "form-entry");
  gtk_box_pack_start(GTK_BOX(chat_entry_and_button),
                     (*main_page).create_chat_data.username, TRUE, TRUE, 0);

  GtkWidget *create_chat_button = gtk_button_new_with_label("✓");
  gtk_box_pack_start(GTK_BOX(chat_entry_and_button),
                     create_chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(create_chat_button),
                              "newchats-button");
  g_signal_connect(create_chat_button, "clicked", G_CALLBACK(chat_creation),
                   main_page);

  (*main_page).create_chat_data.message = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX((*main_page).create_chat_data.form),
                     (*main_page).create_chat_data.message, FALSE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.message),
      "form-message");

  // create group
  (*main_page).group_users_count = 0;
  create_group = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), create_group,
                      "create_group");

  (*main_page).create_group_data.form =
      gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(create_group), (*main_page).create_group_data.form,
                     TRUE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.form),
      "form");
  gtk_widget_set_halign((*main_page).create_group_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).create_group_data.form),
                              450, -1);

  GtkWidget *create_group_label = gtk_label_new("Create new group");
  gtk_style_context_add_class(gtk_widget_get_style_context(create_group_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     create_group_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(create_group_label, GTK_ALIGN_CENTER);

  GtkWidget *group_name_label = gtk_label_new("Group name");
  gtk_style_context_add_class(gtk_widget_get_style_context(group_name_label),
                              "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     group_name_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(group_name_label, GTK_ALIGN_START);

  (*main_page).create_group_data.name = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.name),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     (*main_page).create_group_data.name, FALSE, FALSE, 0);

  GtkWidget *add_user_label = gtk_label_new("Username");
  gtk_style_context_add_class(gtk_widget_get_style_context(add_user_label),
                              "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     add_user_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(add_user_label, GTK_ALIGN_START);

  GtkWidget *group_entry_and_button =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     group_entry_and_button, FALSE, FALSE, 0);

  (*main_page).create_group_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.username),
      "form-entry");
  gtk_box_pack_start(GTK_BOX(group_entry_and_button),
                     (*main_page).create_group_data.username, TRUE, TRUE, 0);

  GtkWidget *add_user_button = gtk_button_new_with_label("+");
  gtk_box_pack_start(GTK_BOX(group_entry_and_button), add_user_button, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(add_user_button),
                              "newchats-button");
  g_signal_connect(add_user_button, "clicked", G_CALLBACK(adding_user),
                   main_page);

  (*main_page).group_box = gtk_flow_box_new();
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     (*main_page).group_box, FALSE, FALSE, 0);

  (*main_page).create_group_data.message = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     (*main_page).create_group_data.message, FALSE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.message),
      "form-message");

  GtkWidget *create_group_button = gtk_button_new_with_label("Create group");
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     create_group_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(create_group_button),
                              "form-button");
  g_signal_connect(create_group_button, "clicked", G_CALLBACK(group_creation),
                   main_page);

  // profile
  user_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), user_info, "user_info");
  (*main_page).profile_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(user_info), (*main_page).profile_data.form, TRUE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).profile_data.form), "form");
  gtk_widget_set_halign((*main_page).profile_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).profile_data.form), 450, -1);
  // avatar
  img_profile = gtk_image_new_from_file("uchat-client/src/gui/resources/rabbit.png");
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile), "img-profile");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), img_profile, FALSE, FALSE, 0);
  gtk_widget_set_halign(img_profile, GTK_ALIGN_CENTER);
  // username
  (*main_page).profile_data.username = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).profile_data.username), "profile-name");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), (*main_page).profile_data.username, FALSE, FALSE, 0);
  // status
  (*main_page).profile_data.status = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).profile_data.status), "status");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), (*main_page).profile_data.status, FALSE, FALSE, 0);
  gtk_widget_set_halign((*main_page).profile_data.status, GTK_ALIGN_CENTER);
  // description
  (*main_page).profile_data.description = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).profile_data.description), "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), (*main_page).profile_data.description, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL((*main_page).profile_data.description), GTK_JUSTIFY_CENTER);
  // change profile details
  GtkWidget *link = gtk_button_new_with_label("Change profile details");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), link, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(link), "profile-link");
  g_signal_connect(link, "clicked", G_CALLBACK(show_edit_page), main_page);

  // edit profile
  edit_profile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), edit_profile, "edit_profile");
  (*main_page).edit_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(edit_profile), (*main_page).edit_data.form, TRUE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).edit_data.form), "form");
  gtk_widget_set_halign((*main_page).edit_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).edit_data.form), 450, -1);
  // label
  GtkWidget *edit_label = gtk_label_new("Change profile details");
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_label), "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), edit_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(edit_label, GTK_ALIGN_CENTER);
  // avatar
  img_profile = gtk_image_new_from_file("uchat-client/src/gui/resources/rabbit.png");
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile), "img-edit");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), img_profile, FALSE, FALSE, 0);
  gtk_widget_set_halign(img_profile, GTK_ALIGN_CENTER);
  // username
  (*main_page).edit_data.username = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).edit_data.username), "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), (*main_page).edit_data.username, FALSE, FALSE, 0);
  // description
  (*main_page).edit_data.description = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).edit_data.description), "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), (*main_page).edit_data.description, FALSE, FALSE, 0);
  // message
  (*main_page).edit_data.message = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).edit_data.message), "form-message");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), (*main_page).edit_data.message, FALSE, FALSE, 0);
  // change profile details
  GtkWidget *button = gtk_button_new_with_label("Change profile details");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(button), "form-button");
  g_signal_connect(button, "clicked", G_CALLBACK(change_profile), main_page);
}
