#include "uchat.h"

int main() {
  char username[50];
  char session_token[65];
  int logged_in = 0;
  char serial_number[64] = {0};

  // Get the device's unique serial number
  get_serial_number(serial_number, sizeof(serial_number));

  // Connect to the server
  int sock = connect_to_server("127.0.0.1", PORT);
  if (sock < 0) {
    printf("Failed to connect to the server.\n");
    return -1;
  }
  printf("Connected to the server successfully.\n");

  // Load the session data
  if (load_session(username, sizeof(username), session_token,
                   sizeof(session_token)) == 0) {
    printf("Loaded session for %s. Checking session status...\n", username);
    logged_in =
        check_session_on_server(sock, username, session_token, serial_number);
  }

  if (!logged_in) {
    printf("Session is invalid or expired. Please log in.\n");
  } else {
    printf("Session is valid. Starting application...\n");
  }

  // Make stdin non-blocking

  // Main loop to handle user actions based on login status
  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int activity = select(sock + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
      perror("select error");
      break;
    }

    // Handle server response
    if (FD_ISSET(sock, &readfds)) {
      handle_response(sock, &logged_in);
    }

    // Handle user input
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      if (logged_in) {
        handle_logged_in_choice(sock, username);
      } else {
        handle_not_logged_in_choice(sock);
      }
    }
  }

  // Reset stdin to blocking mode before exit
  close(sock);
  return 0;
}
