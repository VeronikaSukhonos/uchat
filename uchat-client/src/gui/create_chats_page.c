#include <uchat.h>

void change_mic_image(GtkWidget *mic_button, gpointer data) {
  MicData *mic_data = (MicData *)data;

  if (mic_data->is_active) {
    // Set button image to 'start' and stop recording
    GtkWidget *mic_button_img_start =
        gtk_image_new_from_file(mic_data->img_path_start);
    gtk_button_set_image(GTK_BUTTON(mic_button), mic_button_img_start);
    mic_data->is_active = FALSE;

    // Stop recording
    stop_recording();
  } else {
    // Set button image to 'stop' and start recording
    GtkWidget *mic_button_img_stop =
        gtk_image_new_from_file(mic_data->img_path_stop);
    gtk_button_set_image(GTK_BUTTON(mic_button), mic_button_img_stop);
    mic_data->is_active = TRUE;

    char output_path[100];
    // shoud be in a struct
    int chat_id = 0; // real chat_id from db
    int vmsg_id = 0; // real msg_if from db
    snprintf(output_path, sizeof(output_path), "cache/%d_%d_vmsg.wav", chat_id,
             vmsg_id);
    // Start recording to the specified output path
    start_recording(output_path);
  }
}

void open_close_menu(GtkWidget *menu_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
                                   (*main_page).menu_opened == 1 ? "chats_list"
                                                                 : "menu");
  (*main_page).menu_opened *= -1;
}

void set_selected_button(GtkWidget **selected_button,
                         GtkWidget **new_selected_button) {
  if (*selected_button != NULL)
    gtk_style_context_remove_class(
        gtk_widget_get_style_context(*selected_button), "menu-button-selected");
  *selected_button = *new_selected_button;
  gtk_style_context_add_class(gtk_widget_get_style_context(*selected_button),
                              "menu-button-selected");
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

  GtkWidget *menu_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(sidebar_top), menu_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_button),
                              "dots");
  GtkWidget *menu_button_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/dots.png");
  gtk_button_set_image(GTK_BUTTON(menu_button), menu_button_image);
  g_signal_connect(menu_button, "clicked", G_CALLBACK(open_close_menu),
                   main_page);

  GtkWidget *menu_label = gtk_label_new("Green ❀ Chat");
  gtk_box_pack_start(GTK_BOX(sidebar_top), menu_label, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label),
                              "gchats");
  gtk_widget_set_halign(menu_label, GTK_ALIGN_CENTER);

  // stack
  (*main_page).menu_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(sidebar), (*main_page).menu_stack, FALSE, FALSE,
                     0);
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
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), chats_list_box,
                      "chats_list");

  GtkWidget *chat_button = gtk_button_new();
  GtkWidget *chat_button_label = gtk_label_new("Chat");
  gtk_container_add(GTK_CONTAINER(chat_button), chat_button_label);
  gtk_widget_set_halign(chat_button_label, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(chats_list_box), chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(chat_button),
                              "menu-button");
  g_signal_connect(chat_button, "clicked", G_CALLBACK(show_chat), main_page);
  // central area
  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(chats), (*main_page).central_area_stack, TRUE,
                     TRUE, 0);

  GtkWidget *clear_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), clear_area,
                      "clear_area");

  GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), message_box,
                      "chat");

  (*main_page).chats_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(message_box), (*main_page).chats_stack, TRUE, TRUE,
                     0);

  // stack for chat messages
  GtkWidget *chat_label = gtk_label_new("Chat messages will appear here");
  gtk_stack_add_named(GTK_STACK((*main_page).chats_stack), chat_label, "chat");

  // input box
  GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(message_box), input_box, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(input_box),
                              "input-box");

  // box to hold both the message entry and microphone button
  GtkWidget *message_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(input_box), message_entry_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry_box),
                              "message-entry-box");

  GtkWidget *message_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(message_entry_box), message_scroll,
  					 TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scroll),
  								 GTK_POLICY_NEVER, GTK_POLICY_NEVER);

  GtkWidget *message_entry = gtk_text_view_new();
  GtkTextBuffer *message_buffer =
    gtk_text_view_get_buffer(GTK_TEXT_VIEW(message_entry));
  gtk_container_add(GTK_CONTAINER(message_scroll), message_entry);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(message_entry), GTK_WRAP_WORD_CHAR);
  gtk_widget_set_size_request(message_scroll, -1, 30);
  // HERE MUST BE A PLACEHOLDER
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry),
                              "message-entry");
  g_signal_connect(message_entry, "focus-in-event",
                   G_CALLBACK(change_entry_box_focus), message_entry_box);
  g_signal_connect(message_entry, "focus-out-event",
                   G_CALLBACK(change_entry_box_focus), message_entry_box);
  g_signal_connect(message_buffer, "changed",
  				   G_CALLBACK(check_message_entry_height), message_entry);

  MicData *mic_data = g_new(MicData, 1);
  mic_data->is_active = FALSE;
  mic_data->img_path_start = "uchat-client/src/gui/resources/voice-start.png";
  mic_data->img_path_stop = "uchat-client/src/gui/resources/voice-stop.png";

  GtkWidget *mic_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(message_entry_box), mic_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(mic_button),
                              "mic-button");
  GtkWidget *mic_button_img_start =
      gtk_image_new_from_file(mic_data->img_path_start);
  gtk_button_set_image(GTK_BUTTON(mic_button), mic_button_img_start);
  g_signal_connect(mic_button, "clicked", G_CALLBACK(change_mic_image),
                   mic_data);

  GtkWidget *send_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(input_box), send_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(send_button),
                              "send-button");

  GtkWidget *send_button_img = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/send-button-purple.png");
  gtk_button_set_image(GTK_BUTTON(send_button), send_button_img);
  change_button_hover_image(send_button);
  g_signal_connect(send_button, "clicked", G_CALLBACK(send_message_f),
                   message_buffer);

  // create chat
  create_chat = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), create_chat,
                      "create_chat");
  gtk_style_context_add_class(gtk_widget_get_style_context(create_chat),
                              "main_page_form");

  (*main_page).create_chat_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(create_chat), (*main_page).create_chat_data.form,
                     TRUE, FALSE, 0);
  gtk_widget_set_halign((*main_page).create_chat_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).create_chat_data.form),
                              450, -1);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).create_chat_data.form), "form");

  GtkWidget *create_chat_label = gtk_label_new("New chat");
  gtk_style_context_add_class(gtk_widget_get_style_context(create_chat_label),
                              "form-name-label");
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
  gtk_box_pack_start(GTK_BOX(chat_entry_and_button), create_chat_button, FALSE,
                     FALSE, 0);
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
  gtk_style_context_add_class(gtk_widget_get_style_context(create_group),
                              "main_page_form");

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

  GtkWidget *create_group_label = gtk_label_new("New group");
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
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), user_info,
                      "user_info");
  gtk_style_context_add_class(gtk_widget_get_style_context(user_info),
                              "main_page_form");
  (*main_page).profile_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(user_info), (*main_page).profile_data.form, TRUE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.form), "form");
  gtk_widget_set_halign((*main_page).profile_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).profile_data.form), 450,
                              -1);
  // avatar
  img_profile = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/rabbit_profile.png");
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile),
                              "img-profile");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), img_profile,
                     FALSE, FALSE, 0);
  gtk_widget_set_halign(img_profile, GTK_ALIGN_CENTER);
  // username
  (*main_page).profile_data.username = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.username),
      "profile-name");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
                     (*main_page).profile_data.username, FALSE, FALSE, 0);
  // status
  (*main_page).profile_data.status = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.status), "status");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
                     (*main_page).profile_data.status, FALSE, FALSE, 0);
  gtk_widget_set_halign((*main_page).profile_data.status, GTK_ALIGN_CENTER);
  // Name Surname
  (*main_page).profile_data.name_surname = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.name_surname),
      "profile-name");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
                     (*main_page).profile_data.name_surname, FALSE, FALSE, 0);

  // Student Group
  (*main_page).profile_data.student_group = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.student_group),
      "profile-name");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
                     (*main_page).profile_data.student_group, FALSE, FALSE, 0);
  // description
  (*main_page).profile_data.description = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.description),
      "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
                     (*main_page).profile_data.description, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL((*main_page).profile_data.description),
                        GTK_JUSTIFY_CENTER);
  // change profile details
  GtkWidget *link = gtk_button_new_with_label("Change profile details");
  gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form), link, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(link),
                              "profile-link");
  g_signal_connect(link, "clicked", G_CALLBACK(show_edit_page), main_page);

  // edit profile
  edit_profile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), edit_profile,
                      "edit_profile");
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_profile),
                              "main_page_form");
  (*main_page).edit_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(edit_profile), (*main_page).edit_data.form, TRUE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.form), "form");
  gtk_widget_set_halign((*main_page).edit_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).edit_data.form), 450, -1);
  // label
  GtkWidget *edit_label = gtk_label_new("Change profile details");
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), edit_label, FALSE,
                     FALSE, 0);
  gtk_widget_set_halign(edit_label, GTK_ALIGN_CENTER);
  // avatar
  img_profile = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/rabbit_profile.png");
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile),
                              "img-edit");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), img_profile, FALSE,
                     FALSE, 0);
  gtk_widget_set_halign(img_profile, GTK_ALIGN_CENTER);
  // username
  (*main_page).edit_data.username = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.username),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.username, FALSE, FALSE, 0);
  // Name Surname Entry
  (*main_page).edit_data.name_surname = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.name_surname),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.name_surname, FALSE, FALSE, 0);

  (*main_page).edit_data.student_group = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.student_group),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.student_group, FALSE, FALSE, 0);

  // Student or Teacher
  (*main_page).edit_data.role_combo = gtk_combo_box_text_new();
  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).edit_data.role_combo), "form-role");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "Student");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "Teacher");

  gtk_combo_box_set_active(GTK_COMBO_BOX((*main_page).edit_data.role_combo), 0);

  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), (*main_page).edit_data.role_combo, FALSE, FALSE, 0);
  // message
  (*main_page).edit_data.message = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.message),
      "form-message");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.message, FALSE, FALSE, 0);
  // change profile details
  GtkWidget *button = gtk_button_new_with_label("Change profile details");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), button, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(button),
                              "form-button");
  g_signal_connect(button, "clicked", G_CALLBACK(change_profile), main_page);
}
