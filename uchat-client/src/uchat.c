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

guint main_retry_timeout = 1;   // Start with 1 second intervals for reconnect
guint reconnect_timer_id = 0;   // Timer ID for periodic reconnection
GIOChannel *gio_channel = NULL; // Set retry timeout ID to 0 (no timeout yet)

// Timer function for periodic reconnection attempts
gboolean periodic_reconnection_attempt(gpointer data) {
  attempt_main_reconnection((AppData *)data);
  return TRUE; // Continue the timer until manually stopped on successful
               // reconnect
}

// Additional function definitions and main application setup code
int main(int argc, char *argv[]) {
  gtk_init(&argc, &argv);
  gst_init(&argc, &argv); // Initialize GStreamer
  delete_cache_directory();
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