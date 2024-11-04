#include <uchat.h>

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

void removing_user(GtkWidget *clicked_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  for (int i = 0; i < (*main_page).group_users_count; ++i) {
    if ((*main_page).group_users[i].button == clicked_button) {
      gtk_widget_destroy(gtk_widget_get_parent(clicked_button));
      (*main_page).group_users_count -= 1;
      int j;
      for (j = i; j < (*main_page).group_users_count; ++j) {
        (*main_page).group_users[j].button = (*main_page).group_users[j + 1].button;
        (*main_page).group_users[j].id = (*main_page).group_users[j + 1].id;
      }
      (*main_page).group_users[j].button = NULL;
      (*main_page).group_users[j].id = -1;
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
    if (strcmp(username, "no") == 0) // check is there such user in database
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                         "User does not found");
    else if ((*main_page).group_users_count == USERS_IN_GROUP_COUNT) {
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                         "Maximum number of members is reached");
    }
    else {
      (*main_page).group_users[(*main_page).group_users_count].button = gtk_button_new_with_label(username);
      (*main_page).group_users[(*main_page).group_users_count].id = (*main_page).group_users_count;
      gtk_flow_box_insert(GTK_FLOW_BOX((*main_page).group_box),
                         (*main_page).group_users[(*main_page).group_users_count].button, -1);
      gtk_style_context_add_class(gtk_widget_get_style_context(
                                  (*main_page).group_users[(*main_page).group_users_count].button),
                                  "newchats-users-button");
      g_signal_connect((*main_page).group_users[(*main_page).group_users_count].button,
                       "clicked", G_CALLBACK(removing_user), main_page);
      gtk_widget_set_visible((*main_page).group_users[(*main_page).group_users_count].button, 1);
      (*main_page).group_users_count += 1;
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
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      g_print("%i%c", (*main_page).group_users[i].id, i == (*main_page).group_users_count - 1 ? '\n' : ' ');
    }
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      gtk_widget_destroy(gtk_widget_get_parent((*main_page).group_users[i].button));
      (*main_page).group_users[i].id = -1;
    }
    (*main_page).group_users_count = 0;
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
  //gtk_style_context_add_class(gtk_widget_get_style_context(chats), "box");

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

  GtkWidget *settings_button = gtk_button_new_with_label("Profile");
  gtk_box_pack_start(GTK_BOX(menu_box), settings_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(settings_button),
                              "menu-button");
  g_signal_connect(settings_button, "clicked", G_CALLBACK(show_settings),
                   main_page);

  GtkWidget *log_out_button = gtk_button_new_with_label("Log out");
  gtk_box_pack_start(GTK_BOX(menu_box), log_out_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(log_out_button),
                              "menu-button");
  g_signal_connect(log_out_button, "clicked", G_CALLBACK(log_out), main_page);

  chats_list = gtk_label_new("chats_list");
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), chats_list,
                      "chats_list");

  // central area
  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(chats), (*main_page).central_area_stack, TRUE,
                     FALSE, 0);

  chat = gtk_label_new("chat");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), chat, "chat");

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

  (*main_page).create_group_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(create_group), (*main_page).create_group_data.form,
                     TRUE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.form),
      "form");
  gtk_widget_set_halign((*main_page).create_group_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).create_group_data.form), 450, -1);

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

  GtkWidget *group_entry_and_button = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX((*main_page).create_group_data.form),
                     group_entry_and_button, FALSE, FALSE, 0);

  (*main_page).create_group_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_group_data.username),
      "form-entry");
  gtk_box_pack_start(GTK_BOX(group_entry_and_button),
                     (*main_page).create_group_data.username, TRUE, TRUE, 0);

  GtkWidget *add_user_button = gtk_button_new_with_label("+");
  gtk_box_pack_start(GTK_BOX(group_entry_and_button),
                     add_user_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(add_user_button),
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

  user_info = gtk_label_new("user_info");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), user_info,
                      "user_info");

  edit_profile = gtk_label_new("edit_profile");
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), edit_profile,
                      "edit_profile");
}
