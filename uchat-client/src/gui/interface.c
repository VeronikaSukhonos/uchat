#include "uchat.h"

void setup_gtk_interface(GtkWidget *pages, GtkWidget *registration,
                         GtkWidget *login, GtkWidget *chats,
                         t_form_data *registration_data,
                         t_form_data *login_data, t_main_page_data *main_page) {
  load_css("uchat-client/src/gui/login_registration.css");

  if (logged_in == 1) {
    create_chats_page(pages, chats, main_page);
    create_login_page(pages, login, login_data);
  } else {
    create_login_page(pages, login, login_data);
    create_chats_page(pages, chats, main_page);
  }
  create_registration_page(pages, registration, registration_data);
}

void setup_main_application() {
  main_window = 0;
  setlocale(LC_ALL, "en_US.UTF-8"); //set locale for mac users(idk is it really work) =)
  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  GdkPixbuf *icon = gdk_pixbuf_new_from_file("uchat-client/src/gui/resources/program.png", NULL);
  GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 500, 500, GDK_INTERP_BILINEAR);
  gtk_window_set_icon(GTK_WINDOW(main_window), scaled_icon);
  gtk_window_set_title(GTK_WINDOW(main_window), "Green Chat");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 720);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *pages = gtk_stack_new();
  gtk_container_add(GTK_CONTAINER(main_window), pages);

  GtkWidget *registration, *login, *chats;
  t_form_data registration_data, login_data;
  t_main_page_data main_page;

  login_data.sock = sock;
  registration_data.sock = sock;
  main_page.sock = sock;
  g_print("Connected to the server successfully.%i\n", login_data.sock);

  // Load session data and check status
  char username[64] = {0};
  char serial_number[64] = {0};
  get_serial_number(serial_number, sizeof(serial_number));

  if (load_session(username, sizeof(username), session_token,
                   sizeof(session_token)) == 0) {
    g_print("Loaded session for %s. Checking session status...\n", username);
    logged_in =
        check_session_on_server(sock, username, session_token, serial_number);
  }

  setup_gtk_interface(pages, registration, login, chats, &registration_data,
                      &login_data, &main_page);
  gtk_widget_show_all(main_window);
  gtk_window_set_focus(GTK_WINDOW(main_window), NULL);
  gtk_stack_set_transition_type(GTK_STACK(pages),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  // Set up AppData struct
  AppData app_data = {pages,     registration,       login,
                      chats,     &registration_data, &login_data,
                      &main_page};

  // Monitor the socket with GIOChannel
  if (sock >= 0) {
    gio_channel = g_io_channel_unix_new(sock);
    g_io_add_watch(gio_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, on_server_data,
                   &app_data);
  } else {
    g_print("Failed to set up GIOChannel: invalid socket.\n");
  }

  if (sock < 0) {
    g_print("Socket connection is invalid.\n");
  }
  gtk_main();

  // Clean up
  if (gio_channel) {
    g_io_channel_unref(gio_channel);
  }
  if (main_page.chats != NULL) {
    for (t_chat_node *i = main_page.chats; i != NULL; ) {
      t_chat_node *temp = i->next;
      free(i);
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
    g_print("OnServer\n");

    if (response_status == 0) {
      // Successfully received data, do nothing further
      return TRUE;
    } else {
      g_print("Error in response. Starting reconnection attempts...\n");

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
        reconnect_timer_id = g_timeout_add_seconds(
            main_retry_timeout, periodic_reconnection_attempt, app_data);
      }
    }
  }

  if (condition & (G_IO_HUP | G_IO_ERR)) {
    g_print("Disconnected from server. Starting reconnection attempts...\n");

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
      reconnect_timer_id = g_timeout_add_seconds(
          main_retry_timeout, periodic_reconnection_attempt, app_data);
    }
  }

  return FALSE; // Stop the GIOChannel watch after disconnection
}
