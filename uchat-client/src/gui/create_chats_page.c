#include <uchat.h>

void change_mic_image(GtkWidget *mic_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  if (!main_page || !main_page->opened_chat) {
    g_printerr("Error: Invalid main_page, opened_chat, or mic_data.\n");
    return;
  }

  if (main_page->opened_chat->is_mic_active == TRUE) {
    // Set button image to 'start' and stop recording
    GtkWidget *mic_button_img_start = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-start.png");
    gtk_button_set_image(GTK_BUTTON(mic_button), mic_button_img_start);
    main_page->opened_chat->is_mic_active = FALSE;

    // Stop recording
    stop_recording();
    send_voice_message(main_page->sock, "cache/temp_audio.wav",
                       main_page->opened_chat->id);
  } else {
    // Set button image to 'stop' and start recording
    GtkWidget *mic_button_img_stop = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-stop.png");
    gtk_button_set_image(GTK_BUTTON(mic_button), mic_button_img_stop);
    main_page->opened_chat->is_mic_active = TRUE;

    // Start recording to a temporary file
    start_recording("cache/temp_audio.wav");
  }
}

void open_close_menu(GtkWidget *menu_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Check if any chat has the microphone active
  for (t_chat_node *i = main_page->chats; i != NULL; i = i->next) {
    if (i->chat.is_mic_active) {
      // Stop the microphone recording
      i->chat.is_mic_active = FALSE;

      // Update the mic button image to indicate stopped recording
      GtkWidget *mic_button_img_start = gtk_image_new_from_file(
          "uchat-client/src/gui/resources/voice-start.png");
      gtk_button_set_image(GTK_BUTTON(main_page->mic_button),
                           mic_button_img_start);

      // Log or perform additional actions if needed
      g_print("Recording stopped for chat: %d\n", i->chat.id);
    }
  }

  // Toggle the menu visibility
  stop_recording();
  gtk_stack_set_visible_child_name(GTK_STACK(main_page->menu_stack),
                                   main_page->menu_opened == 1 ? "chats_list"
                                                               : "menu");
  main_page->menu_opened *= -1;
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

  // sidebar
  sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(chats), sidebar, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "menu");
  gtk_widget_set_size_request(GTK_WIDGET(sidebar), 280, -1);

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

  GtkWidget *menu_label_box = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(sidebar_top), menu_label_box, TRUE, TRUE, 0);
  gtk_widget_set_halign(menu_label_box, GTK_ALIGN_CENTER);

  GtkWidget *menu_label1 = gtk_label_new("Green ");
  gtk_grid_attach(GTK_GRID(menu_label_box), menu_label1, 0, 0, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label1),
                              "gchats");

  GtkWidget *menu_label2 =
      gtk_image_new_from_file("uchat-client/src/gui/resources/flower.png");
  gtk_grid_attach(GTK_GRID(menu_label_box), menu_label2, 1, 0, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label2),
                              "gchats");

  GtkWidget *menu_label3 = gtk_label_new(" Chat");
  gtk_grid_attach(GTK_GRID(menu_label_box), menu_label3, 2, 0, 1, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(menu_label3),
                              "gchats");

  // stack
  (*main_page).menu_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(sidebar), (*main_page).menu_stack, TRUE, TRUE, 0);
  (*main_page).menu_opened = 1;
  (*main_page).menu_button_selected = NULL;
  (*main_page).chats = NULL;
  (*main_page).chats_count = 0;

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

  GtkWidget *chats_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), chats_scroll,
                      "chats_list");
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chats_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  (*main_page).chats_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(chats_scroll), (*main_page).chats_box);

  // central area
  (*main_page).central_area_stack = gtk_stack_new();
  gtk_box_pack_start(GTK_BOX(chats), (*main_page).central_area_stack, TRUE,
                     TRUE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).central_area_stack),
      "central-area");

  // clear area
  GtkWidget *clear_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), clear_area,
                      "clear_area");

  GtkWidget *start_label_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(clear_area), start_label_container, TRUE, FALSE,
                     0);
  gtk_widget_set_halign(start_label_container, GTK_ALIGN_CENTER);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(start_label_container),
      "clear-area-container");

  GtkWidget *start_label = gtk_label_new("Select a chat to start messaging");
  gtk_box_pack_start(GTK_BOX(start_label_container), start_label, FALSE, FALSE,
                     0);

  // messages area
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

  // box to hold the message entry, smile button and microphone button
  GtkWidget *message_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(input_box), message_entry_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry_box),
                              "message-entry-box");

  GtkWidget *message_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(message_entry_box), message_scroll, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  GtkWidget *smile_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(message_entry_box), smile_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(smile_button),
                              "input-button");
  GtkWidget *smile_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/smile-open.png");
  gtk_button_set_image(GTK_BUTTON(smile_button), smile_image);
  (*main_page).smile_window = gtk_popover_new(smile_button);
  gtk_widget_set_size_request((*main_page).smile_window, -1, 300);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).smile_window), "smile-window");
  g_signal_connect(smile_button, "clicked", G_CALLBACK(show_smile_menu),
                   (*main_page).smile_window);
  g_signal_connect((*main_page).smile_window, "closed",
                   G_CALLBACK(hide_smile_menu), smile_button);

  GtkWidget *smile_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER((*main_page).smile_window), smile_scroll);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(smile_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  GtkWidget *smile_menu = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(smile_scroll), smile_menu);

  const gchar *emojis[] = {"😀",
                           "😃",
                           "😄",
                           "😁",
                           "😆",
                           "😅",
                           "🤣",
                           "😂",
                           "🙂",
                           "🙃",
                           "😉",
                           "😊",
                           "😇",
                           "🥰",
                           "😍",
                           "🤩",
                           "😘",
                           "😗",
                           "😚",
                           "😙",
                           "🥲",
                           "😋",
                           "😛",
                           "😜",
                           "🤪",
                           "😝",
                           "🤑",
                           "🤗",
                           "🤭",
                           "🤫",
                           "🤔",
                           "🤐",
                           "🤨",
                           "😐",
                           "😑",
                           "😶",
                           "😶‍🌫️",
                           "😏",
                           "😒",
                           "🙄",
                           "😬",
                           "😮‍💨",
                           "🤥",
                           "😌",
                           "😔",
                           "😪",
                           "🤤",
                           "😴",
                           "😷",
                           "🤒",
                           "🤕",
                           "🤢",
                           "🤮",
                           "🤧",
                           "🥵",
                           "🥶",
                           "🥴",
                           "😵",
                           "😵‍💫",
                           "🤯",
                           "🤠",
                           "🥳",
                           "🥸",
                           "😎",
                           "🤓",
                           "🧐",
                           "😕",
                           "😟",
                           "🙁",
                           "😮",
                           "😯",
                           "😲",
                           "😳",
                           "🥺",
                           "😦",
                           "😧",
                           "😨",
                           "😰",
                           "😥",
                           "😢",
                           "😭",
                           "😱",
                           "😖",
                           "😣",
                           "😞",
                           "😓",
                           "😩",
                           "😫",
                           "🥱",
                           "😤",
                           "😡",
                           "😠",
                           "🤬",
                           "😈",
                           "👿",
                           "💀",
                           "💩",
                           "🤡",
                           "👹",
                           "👺",
                           "👻",
                           "👽",
                           "👾",
                           "🤖",
                           "😺",
                           "😸",
                           "😹",
                           "😻",
                           "😼",
                           "😽",
                           "🙀",
                           "😿",
                           "😾",
                           "🙈",
                           "🙉",
                           "🙊",
                           "👋",
                           "🤚",
                           "🖐",
                           "✋",
                           "🖖",
                           "👌",
                           "🤌",
                           "🤏",
                           "✌",
                           "🤞",
                           "🤟",
                           "🤘",
                           "🤙",
                           "👈",
                           "👉",
                           "👆",
                           "👇",
                           "☝",
                           "👍",
                           "👎",
                           "✊",
                           "👊",
                           "🤛",
                           "🤜",
                           "👏",
                           "🙌",
                           "👐",
                           "🤲",
                           "🤝",
                           "🙏",
                           "✍",
                           "💅",
                           "🧠",
                           "👂",
                           "👀",
                           "👁",
                           "👄",
                           "🙋",
                           "🙋‍♂️",
                           "🙋‍♀️",
                           "🤦",
                           "🤦‍♂️",
                           "🤦‍♀️",
                           "🤷",
                           "🤷‍♂️",
                           "🤷‍♀️",
                           "💘",
                           "💕",
                           "❤️‍🔥",
                           "💜",
                           "💚"};

  int emojis_num = sizeof(emojis) / sizeof(emojis[0]);
  GtkWidget *emoji_buttons[emojis_num];
  for (int i = 0; i < emojis_num; i++) {
    emoji_buttons[i] = gtk_button_new_with_label(emojis[i]);
    g_signal_connect(emoji_buttons[i], "clicked",
                     G_CALLBACK(insert_emoji_into_text), main_page);
    gtk_grid_attach(GTK_GRID(smile_menu), emoji_buttons[i], (i % 9), (i / 9), 1,
                    1);
  }

  GtkWidget *message_entry = gtk_text_view_new();
  main_page->message_buffer =
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
  g_signal_connect(main_page->message_buffer, "changed",
                   G_CALLBACK(check_message_entry_height), message_entry);
  g_signal_connect(message_entry, "key-press-event",
                   G_CALLBACK(on_key_press_event), main_page);

  main_page->mic_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(message_entry_box), main_page->mic_button, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(main_page->mic_button), "input-button");

  GtkWidget *mic_start_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/voice-start.png");
  gtk_button_set_image(GTK_BUTTON(main_page->mic_button), mic_start_image);
  g_signal_connect(main_page->mic_button, "clicked",
                   G_CALLBACK(change_mic_image), main_page);

  GtkWidget *send_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(input_box), send_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(send_button),
                              "send-button");

  GtkWidget *send_button_img = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/send-button-purple.png");
  gtk_button_set_image(GTK_BUTTON(send_button), send_button_img);
  change_button_hover_image(send_button);
  g_signal_connect(send_button, "clicked", G_CALLBACK(send_message_f),
                   main_page);

  // create chat
  GtkWidget *create_chat_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      create_chat_scroll, "create_chat");
  create_chat = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(create_chat_scroll), create_chat);
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

  GtkWidget *create_group_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      create_group_scroll, "create_group");
  create_group = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(create_group_scroll), create_group);
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
  GtkWidget *user_info_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      user_info_scroll, "user_info");
  user_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(user_info_scroll), user_info);
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
  //   gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
  //                      (*main_page).profile_data.name_surname, FALSE, FALSE,
  //                      0);

  // Student Group
  (*main_page).profile_data.student_group = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).profile_data.student_group),
      "profile-name");
  //   gtk_box_pack_start(GTK_BOX((*main_page).profile_data.form),
  //                      (*main_page).profile_data.student_group, FALSE, FALSE,
  //                      0);
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
  GtkWidget *edit_profile_scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      edit_profile_scroll, "edit_profile");
  edit_profile = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(edit_profile_scroll), edit_profile);
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

  // choose description
  (*main_page).edit_data.role_combo = gtk_combo_box_text_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.role_combo),
      "form-role");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "Reading PDF");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "Devastated");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "Busy");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), "In progress");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo),
      "Competed all tasks");

  gtk_combo_box_set_active(GTK_COMBO_BOX((*main_page).edit_data.role_combo), 0);

  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.role_combo, FALSE, FALSE, 0);
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

  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "clear_area");
  open_close_menu(NULL, main_page);
}
