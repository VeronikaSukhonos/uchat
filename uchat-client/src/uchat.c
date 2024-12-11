#include "uchat.h"
#include <gst/gst.h>

#define DEFAULT_PORT 5000 // Default port if not provided
#define MAX_PORT 65535
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
int port;
char ip[65] = {0};

guint main_retry_timeout = 10;  // Start with 1 second intervals for reconnect
guint reconnect_timer_id = 0;   // Timer ID for periodic reconnection
GIOChannel *gio_channel = NULL; // Set retry timeout ID to 0 (no timeout yet)

// Timer function for periodic reconnection attempts
gboolean periodic_reconnection_attempt(gpointer data) {
  AppData *app_data = (AppData *)data;
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Reconnecting in %d sec...",
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
void print_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s <server_ip> <port>\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    print_usage(argv[0]);
  }

  // Parse server IP address and port
  const char *server_ip = argv[1];
  char *endptr;
  port = strtol(argv[2], &endptr, 10);

  // Validate the port
  if (*endptr != '\0' || port <= 0 || port > MAX_PORT) {
    fprintf(stderr, "Error: Invalid port number '%s'.\n", argv[2]);
    print_usage(argv[0]);
  }

  strcpy(ip, server_ip);

  // Attempt to connect to the server
  sock = connect_to_server(server_ip, port);

  // Initialize GTK and GStreamer
  gtk_init(&argc, &argv);
  gst_init(&argc, &argv);
  receive_port =
      5000 + (getpid() % 1000); // Generates unique ports like 5001, 5002, etc.
  if (sock < 0) {
    // g_print("Initial connection failed. Showing retry window.\n");
    create_update_failed_window(); // Show retry window if connection fails
  } else {
    // g_print("Connected to the server successfully.\n");
    setup_main_application(); // Open main app if connected
  }
  return 0;
}
