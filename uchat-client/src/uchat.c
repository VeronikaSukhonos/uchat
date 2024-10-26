#include "uchat.h"

int main() {
  char username[50];
  char session_token[65];
  int logged_in = 0;
  char serial_number[64] = {0};

  // Get the device's unique serial number
  get_serial_number(serial_number, sizeof(serial_number));
  // printf("Device Serial Number: %s\n", serial_number);

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

    // Check session validity with the server
    logged_in =
        check_session_on_server(sock, username, session_token, serial_number);
  }

  if (!logged_in) {
    printf("Session is invalid or expired. Please log in.\n");
  } else {
    printf("Session is valid. Starting application...\n");
  }

  // Main loop to handle user actions based on login status
  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);         // Monitor the server socket
    FD_SET(STDIN_FILENO, &readfds); // Monitor standard input (terminal)

    // Wait for activity on either file descriptor
    int activity = select(sock + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0) {
      perror("select error");
      break;
    }

    // If there's data from the server
    if (FD_ISSET(sock, &readfds)) {
      handle_response(sock, &logged_in);
    }

    // If there's input from the terminal
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      int choice;

      if (logged_in) {
        printf("Enter 1 to send a message, 0 to exit: ");
        scanf("%d", &choice);
        getchar(); // Consume newline

        if (choice == 0) {
          printf("Exiting...\n");
          break;
        } else if (choice == 1) {
          // Send message action
          char receiver[50], message[256];
          printf("Enter receiver name: ");
          fgets(receiver, sizeof(receiver), stdin);
          receiver[strcspn(receiver, "\n")] = 0;
          printf("Enter message: ");
          fgets(message, sizeof(message), stdin);
          message[strcspn(message, "\n")] = 0;

          char *json_str = build_json_message(receiver, message);
          send(sock, json_str, strlen(json_str), 0);
          printf("Sent: %s\n", json_str);
          free(json_str);
        } else {
          printf("Invalid choice\n");
        }

      } else {
        printf("Enter 2 to register, 3 to login, 0 to exit: ");
        scanf("%d", &choice);
        getchar(); // Consume newline

        if (choice == 0) {
          printf("Exiting...\n");
          break;
        } else if (choice == 2) {
          // Register action
          char reg_username[20], email[100], password[100];
          printf("Enter username: ");
          fgets(reg_username, sizeof(reg_username), stdin);
          reg_username[strcspn(reg_username, "\n")] = 0;
          printf("Enter email: ");
          fgets(email, sizeof(email), stdin);
          email[strcspn(email, "\n")] = 0;
          printf("Enter password: ");
          fgets(password, sizeof(password), stdin);
          password[strcspn(password, "\n")] = 0;

          char *json_str = build_json_register(reg_username, email, password);
          send(sock, json_str, strlen(json_str), 0);
          printf("Sent: %s\n", json_str);
          free(json_str);

        } else if (choice == 3) {
          // Login action
          char login_username[20], password[100];
          printf("Enter username: ");
          fgets(login_username, sizeof(login_username), stdin);
          login_username[strcspn(login_username, "\n")] = 0;
          printf("Enter password: ");
          fgets(password, sizeof(password), stdin);
          password[strcspn(password, "\n")] = 0;

          char *json_str =
              build_json_login(login_username, password, serial_number);
          send(sock, json_str, strlen(json_str), 0);
          printf("Sent: %s\n", json_str);
          free(json_str);
        } else {
          printf("Invalid choice\n");
        }
      }
    }
  }

  // Clean up and close socket
  close(sock);
  return 0;
}
