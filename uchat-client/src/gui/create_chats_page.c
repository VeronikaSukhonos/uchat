#include <uchat.h>
void send_message_to_server(const gchar *receiver, const gchar *message) {
    // Construct JSON for the message
    cJSON *json_message = cJSON_CreateObject();
    cJSON_AddStringToObject(json_message, "action", "SEND_MESSAGE_TO_USER");
    cJSON_AddStringToObject(json_message, "receiver", receiver);
    cJSON_AddStringToObject(json_message, "message", message);

    char *json_string = cJSON_Print(json_message);
    
    // Here you would send json_string through the socket
    g_print("Sending message to server: %s\n", json_string);

    cJSON_Delete(json_message);  // Clean up
    free(json_string);            // Free the printed string
}
void send_message_f(GtkWidget *widget, gpointer data) {
    GtkEntry *message_entry = GTK_ENTRY(data);
    const gchar *message_text = gtk_entry_get_text(message_entry);
    
    // Assume you have a way to get the current user's username
    const gchar *receiver_username = "current_user"; // Replace this with the actual current username

    if (g_strcmp0(message_text, "") != 0) {
        // Send the message to the server with the receiver's username
        send_message_to_server(receiver_username, message_text);
        gtk_entry_set_text(message_entry, ""); // Clear the entry after sending
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

void show_new_chat(GtkWidget *new_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_chat");
}

void show_new_group(GtkWidget *new_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_group");
}

void show_settings(GtkWidget *settings_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "user_info");
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

void chat_creation(GtkWidget *create_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_chat_data.username));

  if (check_form_data(username, NULL, (*main_page).create_chat_data.message) ==
      1) {
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
}

void adding_user(GtkWidget *add_user_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_group_data.username));

  if (check_form_data(username, NULL, (*main_page).create_group_data.message) ==
      1) {
    if (strcmp(username, "no") == 0) // check is there such user in database
      gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message),
                         "User does not found");
    else {
      g_print("User %s added\n", username);
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
  else {
    g_print("Group %s created\n", name);
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
                                     "chats_list");
    (*main_page).menu_opened = -1;
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                     "chat");
  }
}

void create_chats_page(GtkWidget *pages, GtkWidget *chats,
                       t_main_page_data *main_page) {
  GtkWidget *sidebar;
  GtkWidget *menu_box, *chats_list;
  GtkWidget *create_chat, *create_group, *user_info, *edit_profile, *chat;

  chats = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_stack_add_named(GTK_STACK(pages), chats, "chats");
  gtk_style_context_add_class(gtk_widget_get_style_context(chats), "box");

  // sidebar
  sidebar = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(chats), sidebar, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "menu");

  GtkWidget *menu_button = gtk_button_new_with_label("...");
  gtk_grid_attach(GTK_GRID(sidebar), menu_button, 0, 0, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_button),
                              "dots");
  g_signal_connect(menu_button, "clicked", G_CALLBACK(open_close_menu),
                   main_page);

  GtkWidget *menu_label = gtk_label_new("gChats");
  gtk_grid_attach(GTK_GRID(sidebar), menu_label, 1, 0, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label),
                              "gchats");
  gtk_widget_set_halign(menu_label, GTK_ALIGN_CENTER);

  // stack
  (*main_page).menu_stack = gtk_stack_new();
  gtk_grid_attach(GTK_GRID(sidebar), (*main_page).menu_stack, 0, 1, 2, 1);
  (*main_page).menu_opened = 1;

  menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), menu_box, "menu");

  GtkWidget *new_chat_button = gtk_button_new_with_label("New chat");
  gtk_box_pack_start(GTK_BOX(menu_box), new_chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_chat_button),
                              "menu-button");
  g_signal_connect(new_chat_button, "clicked", G_CALLBACK(show_new_chat),
                   main_page);

  GtkWidget *new_group_button = gtk_button_new_with_label("New group");
  gtk_box_pack_start(GTK_BOX(menu_box), new_group_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_group_button),
                              "menu-button");
  g_signal_connect(new_group_button, "clicked", G_CALLBACK(show_new_group),
                   main_page);

  GtkWidget *settings_button = gtk_button_new_with_label("Settings");
  gtk_box_pack_start(GTK_BOX(menu_box), settings_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(settings_button),
                              "menu-button");
  g_signal_connect(settings_button, "clicked", G_CALLBACK(show_settings),
                   main_page);

  GtkWidget *log_out_button = gtk_button_new_with_label("Log out");
  gtk_box_pack_start(GTK_BOX(menu_box), log_out_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(log_out_button),
                              "menu-button");
  g_signal_connect(log_out_button, "clicked", G_CALLBACK(log_out),
                   main_page); // instead of NULL it must be pages

  chats_list = gtk_label_new("chats_list");
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), chats_list,
                      "chats_list");
  // central area
  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(chats), (*main_page).central_area_stack, FALSE,
                     FALSE, 0);
  chat = gtk_label_new("chat");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), chat, "chat");
  GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_pack_start(GTK_BOX(chats), message_box, TRUE, TRUE, 0);

  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(message_box), (*main_page).central_area_stack, TRUE, TRUE, 0);

  GtkWidget *chat_label = gtk_label_new("chat");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), chat_label, "chat");


  //input box
  GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(input_box), "input-box"); 
  gtk_box_pack_start(GTK_BOX(message_box), input_box, FALSE, FALSE, 5);

  GtkWidget *message_entry = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry), "message-entry");
  gtk_box_pack_start(GTK_BOX(input_box), message_entry, TRUE, TRUE, 5);

  GtkWidget *send_button = gtk_button_new_with_label("Send");
  gtk_style_context_add_class(gtk_widget_get_style_context(send_button), "send-button");
  gtk_box_pack_start(GTK_BOX(input_box), send_button, FALSE, FALSE, 5);

g_signal_connect(send_button, "clicked", G_CALLBACK(send_message_f), message_entry);
  // create chat
  create_chat = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), create_chat,
                      "create_chat");

  (*main_page).create_chat_data.form = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(create_chat), (*main_page).create_chat_data.form,
                     FALSE, FALSE, 50);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.form),
      "newchats");
  gtk_widget_set_halign((*main_page).create_chat_data.form, GTK_ALIGN_CENTER);

  GtkWidget *create_chat_username_label = gtk_label_new("Create new chat");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(create_chat_username_label), "form-label");
  gtk_grid_attach(GTK_GRID((*main_page).create_chat_data.form),
                  create_chat_username_label, 0, 0, 2, 1);
  gtk_widget_set_halign(create_chat_username_label, GTK_ALIGN_START);

  (*main_page).create_chat_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.username),
      "form-entry");
  gtk_grid_attach(GTK_GRID((*main_page).create_chat_data.form),
                  (*main_page).create_chat_data.username, 0, 1, 1, 1);

  GtkWidget *create_chat_button = gtk_button_new_with_label("✓");
  gtk_grid_attach(GTK_GRID((*main_page).create_chat_data.form),
                  create_chat_button, 1, 1, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(create_chat_button),
                              "newchats-button");
  g_signal_connect(create_chat_button, "clicked", G_CALLBACK(chat_creation),
                   main_page);

  (*main_page).create_chat_data.message = gtk_label_new("");
  gtk_grid_attach(GTK_GRID((*main_page).create_chat_data.form),
                  (*main_page).create_chat_data.message, 0, 2, 2, 1);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.message),
      "form-message");

  // create group
  create_group = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), create_group,
                      "create_group");

  (*main_page).create_group_data.form = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(create_group), (*main_page).create_group_data.form,
                     FALSE, FALSE, 50);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.form),
      "newchats");
  gtk_widget_set_halign((*main_page).create_group_data.form, GTK_ALIGN_CENTER);

  GtkWidget *create_group_label = gtk_label_new("Create new group");
  gtk_style_context_add_class(gtk_widget_get_style_context(create_group_label),
                              "newgroup-label");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  create_group_label, 0, 0, 2, 1);
  gtk_widget_set_halign(create_group_label, GTK_ALIGN_START);

  GtkWidget *group_name_label = gtk_label_new("Group name");
  gtk_style_context_add_class(gtk_widget_get_style_context(group_name_label),
                              "form-label");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  group_name_label, 0, 1, 2, 1);
  gtk_widget_set_halign(group_name_label, GTK_ALIGN_START);

  (*main_page).create_group_data.name = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.name),
      "form-entry");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  (*main_page).create_group_data.name, 0, 2, 2, 1);

  GtkWidget *add_user_label = gtk_label_new("Username");
  gtk_style_context_add_class(gtk_widget_get_style_context(add_user_label),
                              "form-label");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form), add_user_label,
                  0, 3, 2, 1);
  gtk_widget_set_halign(add_user_label, GTK_ALIGN_START);

  (*main_page).create_group_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.username),
      "form-entry");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  (*main_page).create_group_data.username, 0, 4, 1, 1);

  GtkWidget *add_user_button = gtk_button_new_with_label("Add");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  add_user_button, 1, 4, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(add_user_button),
                              "newchats-button");
  g_signal_connect(add_user_button, "clicked", G_CALLBACK(adding_user),
                   main_page);

  (*main_page).create_group_data.message = gtk_label_new("");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  (*main_page).create_group_data.message, 0, 5, 2, 1);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.message),
      "form-message");

  GtkWidget *create_group_button = gtk_button_new_with_label("Create group");
  gtk_grid_attach(GTK_GRID((*main_page).create_group_data.form),
                  create_group_button, 0, 6, 2, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(create_group_button),
                              "form-button");
  g_signal_connect(create_group_button, "clicked", G_CALLBACK(group_creation),
                   main_page);

  user_info = gtk_label_new("user_info");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), user_info,
                      "user_info");

  edit_profile = gtk_label_new("edit_profile");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), edit_profile,
                      "edit_profile");
}
