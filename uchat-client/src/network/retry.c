#include "uchat.h"

int attempt_reconnection() {
  sock = connect_to_server(ip, port);
  if (sock >= 0) {
    g_print("Reconnected to server successfully.\n");

    // Ensure retry window and timeout cleanup
    if (retry_window) {
      gtk_widget_destroy(retry_window);
      retry_window = NULL;
    }
    if (retry_timeout_id > 0) {
      g_source_remove(retry_timeout_id);
      retry_timeout_id = 0; // Reset after removal
    }

    retry_timeout = 10; // Reset timeout for future disconnects
    if (main_window == -1)
      setup_main_application(); // Start main app interface
  } else {
    g_print("Retry connection failed.\n");
    retry_timeout = 10;
    retry_timeout *= 2; // Double the timeout for next attempt
    retry_timeout_id = g_timeout_add_seconds(1, retry_connection, NULL);
  }
  return 0;
}

int attempt_main_reconnection(AppData *app_data) {
  sock = connect_to_server(ip, port);
  if (sock >= 0) {
    g_print("Reconnected to server successfully.\n");

    // Stop the periodic reconnection timer
    if (reconnect_timer_id > 0) {
      g_source_remove(reconnect_timer_id);
      reconnect_timer_id = 0;
    }

    // Reset retry timeout for potential future disconnections
    retry_timeout = 1;
    app_data->login_data->sock = sock;
    app_data->registration_data->sock = sock;
    app_data->main_page->sock = sock;

    char serial_number[64] = {0};
    get_serial_number(serial_number, sizeof(serial_number));
    if (app_data->main_page->voice_call_window)
      close_voice_call_window(app_data->main_page->voice_call_window,
                              app_data->main_page);

    if (app_data->main_page->opened_chat != NULL) {
      if (app_data->main_page->opened_chat->is_mic_active == TRUE) {
        stop_recording();
        GtkWidget *mic_button_img_start = gtk_image_new_from_file(
            "uchat-client/src/gui/resources/voice-start.png");
        gtk_button_set_image(GTK_BUTTON(app_data->main_page->mic_button),
                             mic_button_img_start);
        app_data->main_page->opened_chat->is_mic_active = FALSE;
      }
    }
    app_data->main_page->opened_chat = NULL;
    remove_buttons(app_data->main_page);
    remove_all_chat_buttons(app_data->main_page);
    free_message_list(app_data->main_page->messages);
    app_data->main_page->messages = NULL;
    g_print("Message head set to null\n");
    if (load_session(username, sizeof(username), session_token,
                     sizeof(session_token)) == 0) {
      g_print("Loaded session for %s. Checking session status...\n", username);
      logged_in =
          check_session_on_server(sock, username, session_token, serial_number);
    }

    gtk_widget_set_child_visible(app_data->reconnect->box, FALSE);
    gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(app_data->main_overlay),
                                         app_data->reconnect->box, TRUE);

    if (logged_in) {
      gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "chats");
    } else {
      gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "login");
    }

    // Set up the new GIOChannel for the reconnected socket
    gio_channel = g_io_channel_unix_new(sock);
    g_io_add_watch(gio_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, on_server_data,
                   app_data);

  } else {
    main_retry_timeout = 10;
    reconnect_timer_id =
        g_timeout_add_seconds(1, periodic_reconnection_attempt, app_data);
    g_print("Retry connection failed. Retrying again in 10 second...\n");
  }
  return 0;
}
