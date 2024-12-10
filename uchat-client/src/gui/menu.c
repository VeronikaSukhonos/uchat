#include <uchat.h>

void change_mic_image(GtkWidget *mic_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  if (!main_page || !main_page->opened_chat) {
    // g_printerr("Error: Invalid main_page, opened_chat, or mic_data.\n");
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
    // MessageNode *new_message = create_message_node(main_page, VOICE,
    // main_page->opened_chat->id); create_message_button(main_page,
    // new_message);
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
      // g_print("Recording stopped for chat: %d\n", i->chat.id);
    }
  }

  // Toggle the menu visibility
  stop_recording();
  if ((*main_page).chats != NULL && (*main_page).menu_opened == 1) {
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
                                     "chats_list");
    (*main_page).menu_opened = -1;
  } else {
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
                                     "menu");
    (*main_page).menu_opened = 1;
  }
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
  // g_print("Sent: %s\n", json_str);
  g_free(json_str);
  // gtk_stack_set_visible_child_name(GTK_STACK((*GtkWidget)data), "login");
}
