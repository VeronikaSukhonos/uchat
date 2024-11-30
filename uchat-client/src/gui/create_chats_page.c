#include <uchat.h>

void create_chats_page(GtkWidget *pages, GtkWidget *chats,
                       t_main_page_data *main_page) {
  GtkWidget *sidebar;
  GtkWidget *menu_box, *chats_list;
  GtkWidget *create_chat, *create_group, *user_info, *edit_profile, *chat;
  GtkWidget *settings, *edit_password, *support, *email;
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
  (*main_page).messages = NULL;
  (*main_page).chats_count = 0;

  menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).menu_stack), menu_box, "menu");

  GtkWidget *new_chat_button = gtk_button_new();
  GtkWidget *new_chat_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(new_chat_button), new_chat_button_box);
  GtkWidget *new_chat_button_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/new-chat-icon.png");
  gtk_box_pack_start(GTK_BOX(new_chat_button_box), new_chat_button_image, FALSE, FALSE, 0);
  GtkWidget *new_chat_button_label = gtk_label_new("New chat");
  gtk_box_pack_start(GTK_BOX(new_chat_button_box), new_chat_button_label, FALSE, FALSE, 15);
  gtk_box_pack_start(GTK_BOX(menu_box), new_chat_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_chat_button),
                              "menu-button");
  g_signal_connect(new_chat_button, "clicked", G_CALLBACK(show_new_chat),
                   main_page);

  GtkWidget *new_group_button = gtk_button_new();
  GtkWidget *new_group_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(new_group_button), new_group_button_box);
  GtkWidget *new_group_button_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/new-group-icon.png");
  gtk_box_pack_start(GTK_BOX(new_group_button_box), new_group_button_image, FALSE, FALSE, 0);
  GtkWidget *new_group_button_label = gtk_label_new("New group");
  gtk_box_pack_start(GTK_BOX(new_group_button_box), new_group_button_label, FALSE, FALSE, 15);
  gtk_box_pack_start(GTK_BOX(menu_box), new_group_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(new_group_button),
                              "menu-button");
  g_signal_connect(new_group_button, "clicked", G_CALLBACK(show_new_group),
                   main_page);

  GtkWidget *profile_button = gtk_button_new();
  GtkWidget *profile_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(profile_button), profile_button_box);
  GtkWidget *profile_button_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/profile-icon.png");
  gtk_box_pack_start(GTK_BOX(profile_button_box), profile_button_image, FALSE, FALSE, 0);
  GtkWidget *profile_button_label = gtk_label_new("Profile");
  gtk_box_pack_start(GTK_BOX(profile_button_box), profile_button_label, FALSE, FALSE, 15);
  gtk_box_pack_start(GTK_BOX(menu_box), profile_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(profile_button),
                              "menu-button");
  g_signal_connect(profile_button, "clicked", G_CALLBACK(show_profile),
                   main_page);

  GtkWidget *settings_button = gtk_button_new();
  GtkWidget *settings_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(settings_button), settings_button_box);
  GtkWidget *settings_button_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/settings-icon.png");
  gtk_box_pack_start(GTK_BOX(settings_button_box), settings_button_image, FALSE, FALSE, 0);
  GtkWidget *settings_button_label = gtk_label_new("Settings");
  gtk_box_pack_start(GTK_BOX(settings_button_box), settings_button_label, FALSE, FALSE, 15);
  gtk_box_pack_start(GTK_BOX(menu_box), settings_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(settings_button),
                              "menu-button");
  g_signal_connect(settings_button, "clicked", G_CALLBACK(show_settings), main_page);

  GtkWidget *log_out_button = gtk_button_new();
  GtkWidget *log_out_button_label = gtk_label_new("Log out");
  gtk_container_add(GTK_CONTAINER(log_out_button), log_out_button_label);
  gtk_style_context_add_class(gtk_widget_get_style_context(log_out_button), "menu-button");
  gtk_widget_set_halign(log_out_button, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(sidebar), log_out_button, FALSE, FALSE, 8); 
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
  GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack), message_box,
                      "chat");

  // Create profile bar 
  GtkWidget *profile_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start(GTK_BOX(message_box), profile_bar, FALSE, FALSE, 0);
  GtkStyleContext *context = gtk_widget_get_style_context(profile_bar);
  gtk_style_context_add_class(context, "profile-bar");
  gtk_widget_set_size_request(profile_bar, -1, 50);
  gtk_widget_set_halign(profile_bar, GTK_ALIGN_FILL);

  // Add avatar
  GtkWidget *avatar_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(avatar_frame), GTK_SHADOW_NONE);
  gtk_box_pack_start(GTK_BOX(profile_bar), avatar_frame, FALSE, FALSE, 10);

  GtkWidget *avatar = gtk_image_new_from_file("uchat-client/src/gui/resources/rabbit_chats.png");
  gtk_container_add(GTK_CONTAINER(avatar_frame), avatar);
  gtk_widget_set_size_request(avatar, 40, 40);

  // Add nickname
  GtkWidget *nickname_label = gtk_label_new("Nickname");
  gtk_box_pack_start(GTK_BOX(profile_bar), nickname_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(nickname_label), "profile-bar-name");
  (*main_page).chat_nickname = nickname_label;

  // Add spacer to push icons to the right
  GtkWidget *spacer = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(profile_bar), spacer, TRUE, TRUE, 0);

  // Add voice call icon button
  GtkWidget *voice_call_button = gtk_button_new();
  gtk_widget_set_name(voice_call_button, "voice_call_button");
  GtkWidget *voice_call_icon = gtk_image_new_from_file("uchat-client/src/gui/resources/voice-call-start.png");
  gtk_button_set_image(GTK_BUTTON(voice_call_button), voice_call_icon);
  gtk_box_pack_start(GTK_BOX(profile_bar), voice_call_button, FALSE, FALSE, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(voice_call_button), "icon-button");
  g_signal_connect(voice_call_button, "clicked", G_CALLBACK(create_voice_call_window), main_page);
  main_page->voice_call_window = NULL;
  main_page->voice_call_window_label = NULL;
  main_page->profile_window = NULL;

  // Add user icon button
  GtkWidget *user_button = gtk_button_new();
  GtkWidget *user_icon = gtk_image_new_from_file("uchat-client/src/gui/resources/user.png");
  gtk_button_set_image(GTK_BUTTON(user_button), user_icon);
  gtk_box_pack_start(GTK_BOX(profile_bar), user_button, FALSE, FALSE, 5);
  gtk_style_context_add_class(gtk_widget_get_style_context(user_button), "icon-button");
  g_signal_connect(user_button, "clicked", G_CALLBACK(show_participant_profile), main_page);

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

  // box to hold attach button, message entry, smile button and microphone button
  GtkWidget *message_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(input_box), message_entry_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(message_entry_box),
                              "message-entry-box");

  GtkWidget *attach_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(message_entry_box), attach_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(attach_button),
                              "input-button");
  GtkWidget *attach_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/attach-file.png");
  gtk_button_set_image(GTK_BUTTON(attach_button), attach_image);

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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(create_chat_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(create_group_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(user_info_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
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
  GdkPixbuf *original_ava = gdk_pixbuf_new_from_file(
      "uchat-client/src/gui/resources/rabbit_big.png", NULL);
  GdkPixbuf *resized_ava =
      gdk_pixbuf_scale_simple(original_ava, 150, 150, GDK_INTERP_BILINEAR);
  img_profile = gtk_image_new_from_pixbuf(resized_ava);
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile),
                              "img-edit");
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
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(create_group_scroll),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
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
  GtkWidget *edit_label = gtk_label_new("My profile details");
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), edit_label, FALSE,
                     FALSE, 0);
  gtk_widget_set_halign(edit_label, GTK_ALIGN_CENTER);
  // avatar
  img_profile = gtk_image_new_from_pixbuf(resized_ava);
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), img_profile, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(img_profile),
                              "img-edit");
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
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.student_group),
      "before-combo");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.student_group, FALSE, FALSE, 0);

  // choose description
  (*main_page).edit_data.role_combo = gtk_combo_box_text_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.role_combo),
      "form-role");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo),
      " ❔  No status");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), " 📖  Reading PDF");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), " 🧑‍💻  In progress");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), " 💯  Assesment");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), " 😫  Devastated");
  gtk_combo_box_text_append_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo), " 🧠  Genius");

  gtk_combo_box_set_active(GTK_COMBO_BOX((*main_page).edit_data.role_combo), 0);

  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.role_combo, FALSE, FALSE, 0);
  (*main_page).edit_data.message = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).edit_data.message),
      "form-message");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form),
                     (*main_page).edit_data.message, FALSE, FALSE, 0);
  GtkWidget *button = gtk_button_new_with_label("Change profile details");
  gtk_box_pack_start(GTK_BOX((*main_page).edit_data.form), button, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(button),
                              "form-button");
  g_signal_connect(button, "clicked", G_CALLBACK(change_profile), main_page);

  // settings
  GtkWidget *settings_page = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      settings_page, "settings");
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(settings_page),
                                 GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  settings = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(settings_page), settings);
  gtk_style_context_add_class(gtk_widget_get_style_context(settings),
                              "main_page_form");
  (*main_page).settings_data.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(settings), (*main_page).settings_data.form, TRUE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).settings_data.form), "form");
  gtk_widget_set_halign((*main_page).settings_data.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).settings_data.form), 450, -1);

  GtkWidget *settings_label = gtk_label_new("Settings");
  gtk_style_context_add_class(gtk_widget_get_style_context(settings_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).settings_data.form), settings_label, FALSE,
                     FALSE, 0);
  gtk_widget_set_halign(settings_label, GTK_ALIGN_CENTER);

//   GtkWidget *link_pw_change = gtk_button_new_with_label("Change password");
//   gtk_box_pack_start(GTK_BOX((*main_page).settings_data.form), link_pw_change, FALSE,
//                      FALSE, 0);
//   gtk_style_context_add_class(gtk_widget_get_style_context(link_pw_change),
//                               "settings-link");
//   g_signal_connect(link_pw_change, "clicked", G_CALLBACK(show_pw), main_page);

  GtkWidget *link_email = gtk_button_new_with_label("Change email");
  gtk_box_pack_start(GTK_BOX((*main_page).settings_data.form), link_email, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(link_email),
                              "settings-link");
  g_signal_connect(link_email, "clicked", G_CALLBACK(show_email), main_page);

  // support
  GtkWidget *link_support = gtk_button_new_with_label("Support");
  gtk_box_pack_start(GTK_BOX((*main_page).settings_data.form), link_support, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(link_support),
                              "settings-link");
  g_signal_connect(link_support, "clicked", G_CALLBACK(show_support), main_page);

  // change pw
  GtkWidget *pw_change_page = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      pw_change_page, "edit_password");
  edit_password = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(pw_change_page), edit_password);
  gtk_style_context_add_class(gtk_widget_get_style_context(edit_password),
                              "main_page_form");
  (*main_page).change_pw.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(edit_password), (*main_page).change_pw.form, TRUE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).change_pw.form), "form");
  gtk_widget_set_halign((*main_page).change_pw.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).change_pw.form), 450, -1);
  GtkWidget *change_pw_label = gtk_label_new("Change password");
  gtk_style_context_add_class(gtk_widget_get_style_context(change_pw_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).change_pw.form), change_pw_label, FALSE,
                     FALSE, 0);
  gtk_widget_set_halign(change_pw_label, GTK_ALIGN_CENTER);

  (*main_page).change_pw.old_pw = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).change_pw.old_pw),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).change_pw.form),
                     (*main_page).change_pw.old_pw, FALSE, FALSE, 0);
  (*main_page).change_pw.new_pw = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).change_pw.new_pw),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).change_pw.form),
                     (*main_page).change_pw.new_pw, FALSE, FALSE, 0);
  (*main_page).change_pw.new_pw_again = gtk_entry_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).change_pw.new_pw_again),
      "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).change_pw.form),
                     (*main_page).change_pw.new_pw_again, FALSE, FALSE, 0);
  GtkWidget *change_pw_button = gtk_button_new_with_label("Submit");
  gtk_box_pack_start(GTK_BOX((*main_page).change_pw.form), change_pw_button, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(change_pw_button),
                              "form-button");
  g_signal_connect(change_pw_button, "clicked", G_CALLBACK(change_password), main_page);

  // Email page setup
  GtkWidget *email_page = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      email_page, "email");

  email = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(email_page), email);
  gtk_style_context_add_class(gtk_widget_get_style_context(email), "main_page_form");

  (*main_page).email_change.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(email), (*main_page).email_change.form, TRUE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).email_change.form), "form");
  gtk_widget_set_halign((*main_page).email_change.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).email_change.form), 450, -1);
  GtkWidget *email_label = gtk_label_new("Change email");
  gtk_style_context_add_class(gtk_widget_get_style_context(email_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).email_change.form), email_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(email_label, GTK_ALIGN_CENTER);
  GtkWidget *enter_email_label = gtk_label_new("Enter new email:");
  gtk_style_context_add_class(gtk_widget_get_style_context(enter_email_label), "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).email_change.form), enter_email_label, FALSE, FALSE, 5);
  GtkWidget *email_entry = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(email_entry), "form-entry");
  gtk_box_pack_start(GTK_BOX((*main_page).email_change.form), email_entry, FALSE, FALSE, 5);
  (*main_page).email_change.email = email_entry;
  GtkWidget *email_change_button = gtk_button_new_with_label("Submit");
  gtk_box_pack_start(GTK_BOX((*main_page).email_change.form), email_change_button, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(email_change_button),
                              "form-button");
  g_signal_connect(email_change_button, "clicked", G_CALLBACK(change_email), main_page);
  (*main_page).email_change.message = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).email_change.message),
      "form-message");
  gtk_box_pack_start(GTK_BOX((*main_page).email_change.form),
                     (*main_page).email_change.message, FALSE, FALSE, 0);

  // Support page setup
  GtkWidget *support_page = gtk_scrolled_window_new(NULL, NULL);
  gtk_stack_add_named(GTK_STACK((*main_page).central_area_stack),
                      support_page, "support");

  support = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(support_page), support);
  gtk_style_context_add_class(gtk_widget_get_style_context(support), "main_page_form");

  (*main_page).support.form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(support), (*main_page).support.form, TRUE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).support.form), "form");
  gtk_widget_set_halign((*main_page).support.form, GTK_ALIGN_CENTER);
  gtk_widget_set_size_request(GTK_WIDGET((*main_page).support.form), 450, -1);
  GtkWidget *support_label = gtk_label_new("Support");
  gtk_style_context_add_class(gtk_widget_get_style_context(support_label),
                              "form-name-label");
  gtk_box_pack_start(GTK_BOX((*main_page).support.form), support_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(support_label, GTK_ALIGN_CENTER);
  GtkWidget *title_label = gtk_label_new("Subject:");
  gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "form-label");
  gtk_box_pack_start(GTK_BOX((*main_page).support.form), title_label, FALSE, FALSE, 5);
  (*main_page).support.subject_combo = gtk_combo_box_text_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).support.subject_combo),
      "form-role");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).support.subject_combo), "Forgot password");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).support.subject_combo), "Account issue");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).support.subject_combo), "Technical support");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).support.subject_combo), "General inquiry");
  gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT((*main_page).support.subject_combo), "Other");
  gtk_combo_box_set_active(GTK_COMBO_BOX((*main_page).support.subject_combo), 0);
  gtk_box_pack_start(GTK_BOX((*main_page).support.form), (*main_page).support.subject_combo, FALSE, FALSE, 0);

  GtkWidget *description_entry = gtk_entry_new();
  gtk_widget_set_size_request(description_entry, 450, 50);
  gtk_entry_set_placeholder_text(GTK_ENTRY(description_entry), "Describe your problem here...");
  gtk_style_context_add_class(gtk_widget_get_style_context(description_entry), "support-description-entry");

  gtk_box_pack_start(GTK_BOX((*main_page).support.form), description_entry, FALSE, FALSE, 5);
  (*main_page).support.support_request = description_entry;
  GtkWidget *support_button = gtk_button_new_with_label("Submit");
  gtk_box_pack_start(GTK_BOX((*main_page).support.form), support_button, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(support_button),
                              "form-button");
  g_signal_connect(support_button, "clicked", G_CALLBACK(send_support_request), main_page);
  (*main_page).support.message = gtk_label_new("");
  gtk_style_context_add_class(
      gtk_widget_get_style_context((*main_page).support.message),
      "form-message");
  gtk_box_pack_start(GTK_BOX((*main_page).support.form),
                     (*main_page).support.message, FALSE, FALSE, 0);

  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "clear_area");
 
  open_close_menu(NULL, main_page);
}
