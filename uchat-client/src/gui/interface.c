#include "uchat.h"

void close_main_window(GtkWidget *main_window, t_main_page_data *main_page) {
  gtk_drag_dest_set(main_page->message_entry, GTK_DEST_DEFAULT_ALL, NULL, 0,
                    GDK_ACTION_COPY);
  gtk_widget_destroy((*main_page).smile_window);
  gtk_main_quit();
}

void setup_gtk_interface(GtkWidget *pages, GtkWidget *registration,
                         GtkWidget *login, GtkWidget *chats,
                         t_form_data *registration_data,
                         t_form_data *login_data, t_main_page_data *main_page,
                         GtkWidget *main_overlay,
                         t_reconnect_message *reconnect) {
  load_css("uchat-client/src/gui/login_registration.css");
  load_css("uchat-client/src/gui/profile_bar.css");

  if (logged_in == 1) {
    create_chats_page(pages, chats, main_page);
    create_login_page(pages, login, login_data);
  } else {
    create_login_page(pages, login, login_data);
    create_chats_page(pages, chats, main_page);
  }
  create_registration_page(pages, registration, registration_data);
  create_reconnect_message(main_overlay, reconnect);
}

void setup_main_application() {
  main_window = 0;
  setlocale(
      LC_ALL,
      "en_US.UTF-8"); // set locale for mac users(idk is it really work) =)
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GdkPixbuf *icon = gdk_pixbuf_new_from_file(
      "uchat-client/src/gui/resources/program.png", NULL);
  GdkPixbuf *scaled_icon =
      gdk_pixbuf_scale_simple(icon, 500, 500, GDK_INTERP_BILINEAR);
  gtk_window_set_icon(GTK_WINDOW(main_window), scaled_icon);
  gtk_window_set_title(GTK_WINDOW(main_window), "Green Chat");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 720);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  GtkWidget *main_overlay = gtk_overlay_new();
  gtk_container_add(GTK_CONTAINER(main_window), main_overlay);

  GtkWidget *pages = gtk_stack_new();
  gtk_container_add(GTK_CONTAINER(main_overlay), pages);

  GtkWidget *registration, *login, *chats;
  t_form_data registration_data, login_data;
  t_main_page_data main_page;
  t_reconnect_message reconnect;

  login_data.sock = sock;
  registration_data.sock = sock;
  main_page.sock = sock;
  // g_print("Connected to the server successfully.%i\n", login_data.sock);

  // Load session data and check status
  char serial_number[64] = {0};
  get_serial_number(serial_number, sizeof(serial_number));

  if (load_session(username, sizeof(username), session_token,
                   sizeof(session_token)) == 0) {
    // g_print("Loaded session for %s. Checking session status...\n", username);
    logged_in =
        check_session_on_server(sock, username, session_token, serial_number);
  }

  setup_gtk_interface(pages, registration, login, chats, &registration_data,
                      &login_data, &main_page, main_overlay, &reconnect);
  g_signal_connect(main_window, "destroy", G_CALLBACK(close_main_window),
                   &main_page);
  gtk_widget_show_all(main_window);
  gtk_window_set_focus(GTK_WINDOW(main_window), NULL);
  gtk_stack_set_transition_type(GTK_STACK(pages),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  // Set up AppData struct
  AppData app_data = {pages,      registration,       login,
                      chats,      &registration_data, &login_data,
                      &main_page, main_overlay,       &reconnect};
  app_data.main_page->opened_chat = NULL;

  // Monitor the socket with GIOChannel
  if (sock >= 0) {
    gio_channel = g_io_channel_unix_new(sock);
    g_io_add_watch(gio_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, on_server_data,
                   &app_data);
  } else {
    // g_print("Failed to set up GIOChannel: invalid socket.\n");
  }

  if (sock < 0) {
    // g_print("Socket connection is invalid.\n");
  }
  gtk_main();

  // Clean up
  if (gio_channel) {
    g_io_channel_unref(gio_channel);
  }
  if (main_page.chats != NULL) {
    for (t_chat_node *i = main_page.chats; i != NULL;) {
      t_chat_node *temp = i->next;
      g_free(i);
      i = temp;
    }
  }
  if (main_page.messages != NULL) {
    for (MessageNode *i = main_page.messages; i != NULL;) {
      MessageNode *temp = i->next;
      g_free(i);
      i = temp;
    }
  }
  close(sock);
}

gboolean on_server_data(GIOChannel *source, GIOCondition condition,
                        gpointer data) {
  AppData *app_data = (AppData *)data;

  if (condition & G_IO_IN) {
    int response_status = handle_response(sock, &logged_in, app_data);
    // g_print("OnServer\n");

    if (response_status == 0) {
      // Successfully received data, do nothing further
      // g_print("Success\n");
      return TRUE;
    } else {
      if (app_data->main_page->voice_call_window)
        close_voice_call_window(app_data->main_page->voice_call_window,
                                app_data->main_page);
      gtk_widget_set_child_visible(app_data->reconnect->box, TRUE);
      gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(app_data->main_overlay),
                                           app_data->reconnect->box, FALSE);
      // g_print("Error in response. Starting reconnection attempts...\n");

      // Close the socket and remove the GIOChannel
      if (gio_channel != NULL) {
        g_io_channel_unref(gio_channel);
        gio_channel = NULL;
      }
      close(sock);

      // Set the socket to -1 to indicate disconnection
      sock = -1;
      app_data->login_data->sock = sock;
      app_data->registration_data->sock = sock;
      app_data->main_page->sock = sock;

      // Start periodic reconnection attempts if not already started
      if (reconnect_timer_id == 0) {
        reconnect_timer_id =
            g_timeout_add_seconds(1, periodic_reconnection_attempt, app_data);
      }
    }
  }

  if (condition & (G_IO_HUP | G_IO_ERR)) {
    if (app_data->main_page->voice_call_window)
      close_voice_call_window(app_data->main_page->voice_call_window,
                              app_data->main_page);
    gtk_widget_set_child_visible(app_data->reconnect->box, TRUE);
    gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(app_data->main_overlay),
                                         app_data->reconnect->box, FALSE);
    // g_print("Disconnected from server. Starting reconnection attempts...\n");

    // Close the GIOChannel if the connection was lost
    if (gio_channel != NULL) {
      g_io_channel_unref(gio_channel);
      gio_channel = NULL;
    }
    close(sock);
    sock = -1;
    app_data->login_data->sock = sock;
    app_data->registration_data->sock = sock;
    app_data->main_page->sock = sock;

    // Start periodic reconnection attempts
    if (reconnect_timer_id == 0) {
      reconnect_timer_id =
          g_timeout_add_seconds(1, periodic_reconnection_attempt, app_data);
    }
  }

  return FALSE; // Stop the GIOChannel watch after disconnection
}
