#include <stdio.h>
#include <uchat.h>
#include <unistd.h>

#define INPUT_BUFFER_SIZE 1024

// Function to handle user choices when logged in
void handle_logged_in_choice(int sock, const char *username) {
  if (!username)
    return;

  char buffer[INPUT_BUFFER_SIZE];
  printf("Options: 2 to create private chat, 3 to create group chat, 4 to view "
         "chat list, 0 to exit:\n");
  fflush(stdout);

  // Use read() instead of scanf
  int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
  if (bytes_read > 0) {
    buffer[bytes_read - 1] = '\0';
    if (strcmp(buffer, "\0") == 0) {
      return;
    } // Remove newline character
    int choice = atoi(buffer); // Convert to int

    switch (choice) {
    case 0:
      printf("Exiting...\n");
      exit(0);
    case 2: {
      printf("Enter username for private chat: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
      if (bytes_read > 0) {
        buffer[bytes_read - 1] = '\0'; // Remove newline
        cJSON *json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "action", "CREATE_CHAT");
        cJSON_AddStringToObject(json, "username", buffer);
        char *json_str = cJSON_Print(json);
        cJSON_Delete(json);
        send(sock, json_str, strlen(json_str), 0);
        printf("Sent: %s\n", json_str);
        free(json_str);
      }
      break;
    }
    case 3: {
      printf("Enter group chat name: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
      if (bytes_read > 0) {
        buffer[bytes_read - 1] = '\0';
        char chat_name[50];
        strncpy(chat_name, buffer, sizeof(chat_name) - 1);
        printf("Enter number of users to add to the group: ");
        fflush(stdout);
        bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        int num_users = atoi(buffer);

        char usernames[num_users][50];
        for (int i = 0; i < num_users; i++) {
          printf("Enter username %d: ", i + 1);
          fflush(stdout);
          bytes_read =
              read(STDIN_FILENO, usernames[i], sizeof(usernames[i]) - 1);
          usernames[i][bytes_read - 1] = '\0'; // Remove newline
        }
        char *json_str = build_json_group_chat(chat_name, usernames, num_users);
        send(sock, json_str, strlen(json_str), 0);
        printf("Sent: %s\n", json_str);
        free(json_str);
      }
      break;
    }
    case 4: {
      cJSON *json = cJSON_CreateObject();
      cJSON_AddStringToObject(json, "action", "GET_CHAT_LIST");
      char *json_str = cJSON_Print(json);
      cJSON_Delete(json);
      send(sock, json_str, strlen(json_str), 0);
      printf("Sent request for chat list: %s\n", json_str);
      free(json_str);
      break;
    }
    default:
      printf("Invalid choice\n");
      break;
    }
  }
}

// Function to handle user choices when not logged in
void handle_not_logged_in_choice(int sock) {
  char buffer[INPUT_BUFFER_SIZE];
  printf("Options: 1 to register, 2 to login, 0 to exit:\n");
  fflush(stdout);

  int bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
  if (bytes_read > 0) {
    buffer[bytes_read - 1] = '\0'; // Remove newline character
    int choice = atoi(buffer);

    switch (choice) {
    case 0:
      printf("Exiting...\n");
      exit(0);
    case 1: {
      char reg_username[20], email[100], password[100];
      printf("Enter username: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, reg_username, sizeof(reg_username) - 1);
      reg_username[bytes_read - 1] = '\0';

      printf("Enter email: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, email, sizeof(email) - 1);
      email[bytes_read - 1] = '\0';

      printf("Enter password: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, password, sizeof(password) - 1);
      password[bytes_read - 1] = '\0';

      char *json_str = build_json_register(reg_username, email, password);
      send(sock, json_str, strlen(json_str), 0);
      printf("Sent: %s\n", json_str);
      free(json_str);
      break;
    }
    case 2: {
      char login_username[20], password[100];
      printf("Enter username: ");
      fflush(stdout);
      bytes_read =
          read(STDIN_FILENO, login_username, sizeof(login_username) - 1);
      login_username[bytes_read - 1] = '\0';

      printf("Enter password: ");
      fflush(stdout);
      bytes_read = read(STDIN_FILENO, password, sizeof(password) - 1);
      password[bytes_read - 1] = '\0';

      char serial_number[256] = {0};
      get_serial_number(serial_number, sizeof(serial_number));
      char *json_str =
          build_json_login(login_username, password, serial_number);
      send(sock, json_str, strlen(json_str), 0);
      printf("Sent: %s\n", json_str);
      free(json_str);
      break;
    }
    default:
      printf("Invalid choice\n");
      break;
    }
  }
}
