#include <glib.h>
#include <gtk/gtk.h>
#include <uchat.h>
int sock;
int logged_in = 0;
char session_token[65];

// Struct to store pointers to the widgets and form data

// Function to handle server data
gboolean on_server_data(GIOChannel *source, GIOCondition condition,
                        gpointer data) {
  AppData *app_data = (AppData *)data;

  if (condition & G_IO_IN) {
    if (handle_response(sock, &logged_in, app_data) == 1) {
      gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "chats");
    }
  }

  if (condition & (G_IO_HUP | G_IO_ERR)) {
    g_print("Disconnected from server. Attempting to reconnect...\n");
    g_io_channel_unref(source);
    close(sock);
    // sock = attempt_reconnect();
    // if (sock > 0) {
    //   GIOChannel *new_channel = g_io_channel_unix_new(sock);
    //   g_io_add_watch(new_channel, G_IO_IN | G_IO_HUP | G_IO_ERR,
    //   on_server_data,
    //                  app_data);
    // } else {
    //   gtk_main_quit();
    //   return FALSE;
    // }
  }
  return TRUE;
}

// Initialize GTK interface and load different views
void setup_gtk_interface(GtkWidget *pages, GtkWidget *registration,
                         GtkWidget *login, GtkWidget *chats,
                         t_form_data *registration_data,
                         t_form_data *login_data) {

  load_css("uchat-client/src/gui/login_registration.css");

  // Use logged_in flag to set the starting page
  if (logged_in == 1) {
    create_chats_page(pages, chats);
    create_login_page(pages, login, login_data);
  } else {
    create_login_page(pages, login, login_data);
    create_chats_page(pages, chats);
  }
  create_registration_page(pages, registration, registration_data);
}

// Main function
int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);

  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window), "Green Chat");
  gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 450);
  g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *pages = gtk_stack_new();
  gtk_container_add(GTK_CONTAINER(main_window), pages);

  // Declare the pages and form data structures here
  GtkWidget *registration, *login, *chats;
  t_form_data registration_data, login_data;

  // Connect to the server
  sock = connect_to_server("127.0.0.1", PORT);
  if (sock < 0) {
    g_print("Failed to connect to the server.\n");
    return -1;
  }
  login_data.sock = sock;
  registration_data.sock = sock;
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

  // Initialize GTK interface
  setup_gtk_interface(pages, registration, login, chats, &registration_data,
                      &login_data);
  gtk_widget_show_all(main_window);
  gtk_stack_set_transition_type(GTK_STACK(pages),
                                GTK_STACK_TRANSITION_TYPE_CROSSFADE);

  // Set up AppData struct
  AppData app_data = {pages, registration,       login,
                      chats, &registration_data, &login_data};

  // Monitor the socket with GIOChannel
  GIOChannel *gio_channel = g_io_channel_unix_new(sock);
  g_io_add_watch(gio_channel, G_IO_IN | G_IO_HUP | G_IO_ERR, on_server_data,
                 &app_data);

  gtk_main();

  // Clean up
  g_io_channel_unref(gio_channel);
  close(sock);
  return 0;
}