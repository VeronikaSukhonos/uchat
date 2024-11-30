#include "uchat.h"
#include <gst/gst.h>

// Global variable definitions
int sock = -1;                  // Initialize to an invalid socket by default
int logged_in = 0;              // Set initial logged-in status to false (0)
char session_token[65] = {0};   // Initialize session token to an empty string
int main_window = -1;           // Set main window flag to an invalid state
GtkWidget *retry_window = NULL; // Set retry window pointer to NULL initially
GtkWidget *retry_label = NULL;  // Set retry label pointer to NULL initially
guint retry_timeout = 10;       // Set initial retry timeout to 10 seconds
guint retry_timeout_id = 0;
char username[64] = {0};
int in_call = 0;
int receive_port = 0; // Tracks if the user is in a call
int is_calling = 0;

guint main_retry_timeout = 10;  // Start with 1 second intervals for reconnect
guint reconnect_timer_id = 0;   // Timer ID for periodic reconnection
GIOChannel *gio_channel = NULL; // Set retry timeout ID to 0 (no timeout yet)

// Timer function for periodic reconnection attempts
gboolean periodic_reconnection_attempt(gpointer data) {
  AppData *app_data = (AppData *)data;
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Retrying in %d seconds...",
           main_retry_timeout);
  gtk_label_set_text(GTK_LABEL(app_data->reconnect->label), buffer);

  if (main_retry_timeout == 0) {
    attempt_main_reconnection(app_data);
    return FALSE; // Stop the timeout if reconnection is attempted
  } else {
    main_retry_timeout--;
  }
  return TRUE; // Continue countdown
}

// Additional function definitions and main application setup code
int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  gst_init(&argc, &argv); // Initialize GStreamer
  receive_port =
      5000 + (getpid() % 1000); // Generates unique ports like 5001, 5002, etc.
  // Attempt initial connection to the server
  sock = connect_to_server("127.0.0.1", PORT);
  if (sock < 0) {
    g_print("Initial connection failed. Showing retry window.\n");
    create_update_failed_window(); // Show retry window if connection fails
  } else {
    g_print("Connected to the server successfully.\n");
    setup_main_application(); // Open main app if connected
  }
  return 0;
}