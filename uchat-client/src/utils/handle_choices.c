#include <uchat.h>

// Function to handle user choices when logged in
void handle_logged_in_choice(int sock, const char *username) {
  if (username) {
  }
  int choice;
  printf("Enter 1 to send a message, 2 to create private chat, 3 to create "
         "group chat, 4 to view chat list, 0 to exit: ");
  scanf("%d", &choice);
  getchar();

  if (choice == 0) {
    printf("Exiting...\n");
    exit(0);
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

  } else if (choice == 2) {
    // Create private chat action
    char private_username[20];
    printf("Enter username for private chat: ");
    fgets(private_username, sizeof(private_username), stdin);
    private_username[strcspn(private_username, "\n")] = 0;

    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "CREATE_CHAT");
    cJSON_AddStringToObject(json, "username", private_username);
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);

    send(sock, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    free(json_str);

  } else if (choice == 3) {
    // Create group chat action
    char chat_name[50];
    int num_users;
    printf("Enter group chat name: ");
    fgets(chat_name, sizeof(chat_name), stdin);
    chat_name[strcspn(chat_name, "\n")] = 0;

    printf("Enter the number of users to add to the group: ");
    scanf("%d", &num_users);
    getchar(); // Consume newline

    // Collect usernames
    char usernames[num_users][50];
    for (int i = 0; i < num_users; i++) {
      printf("Enter username %d: ", i + 1);
      fgets(usernames[i], sizeof(usernames[i]), stdin);
      usernames[i][strcspn(usernames[i], "\n")] = 0;
    }

    // Build and send the group chat JSON
    char *json_str = build_json_group_chat(chat_name, usernames, num_users);
    send(sock, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    free(json_str);

  } else if (choice == 4) {
    // Request chat list from server
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "GET_CHAT_LIST");
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);

    send(sock, json_str, strlen(json_str), 0);
    printf("Sent request for chat list: %s\n", json_str);
    free(json_str);
  } else {
    printf("Invalid choice\n");
  }
}

// Function to handle user choices when not logged in
void handle_not_logged_in_choice(int sock) {
  int choice;
  printf("Enter 1 to register, 2 to login, 0 to exit: ");
  scanf("%d", &choice);
  getchar(); // Consume newline

  if (choice == 0) {
    printf("Exiting...\n");
    exit(0);
  } else if (choice == 1) {
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

  } else if (choice == 2) {
    // Login action
    char login_username[20], password[100];
    printf("Enter username: ");
    fgets(login_username, sizeof(login_username), stdin);
    login_username[strcspn(login_username, "\n")] = 0;
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    char serial_number[256] = {0};

    // Get the device's unique serial number
    get_serial_number(serial_number, sizeof(serial_number));
    char *json_str = build_json_login(login_username, password, serial_number);
    send(sock, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    free(json_str);
  } else {
    printf("Invalid choice\n");
  }
}