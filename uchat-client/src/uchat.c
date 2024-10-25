#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "cJSON.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    // Setup server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed\n");
        return -1;
    }

    // Main loop to handle input and server responses
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);          // Monitor the server socket
        FD_SET(STDIN_FILENO, &readfds);  // Monitor standard input (terminal)

        // Wait for activity on either file descriptor
        int activity = select(sock + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            break;
        }

        // If there's data from the server
        if (FD_ISSET(sock, &readfds)) {
            int valread = read(sock, buffer, sizeof(buffer));
            if (valread > 0) {
                buffer[valread] = '\0';
                printf("Server response: %s\n", buffer);
            } else if (valread == 0) {
                printf("Server disconnected\n");
                break;
            }
        }

        // If there's input from the terminal
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int choice;
            printf("Enter 1 to send a message, 2 to login: ");
            scanf("%d", &choice);
            getchar();  // Consume newline

            // Create a JSON object
            cJSON *json = cJSON_CreateObject();

            switch (choice) {
                case 1: {
                    char receiver[50];
                    char message[256];

                    // Gather receiver's name and message
                    printf("Enter receiver name: ");
                    fgets(receiver, sizeof(receiver), stdin);
                    receiver[strcspn(receiver, "\n")] = 0;  // Remove newline character

                    printf("Enter message: ");
                    fgets(message, sizeof(message), stdin);
                    message[strcspn(message, "\n")] = 0;  // Remove newline character

                    // Construct JSON message
                    cJSON_AddStringToObject(json, "action", "SEND_MESSAGE_TO_USER");
                    cJSON_AddStringToObject(json, "receiver", receiver);
                    cJSON_AddStringToObject(json, "message", message);
                    break;
                }
                case 2: {
                    // Construct JSON login
                    char username[20];
                    printf("Enter name: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = 0;
                    cJSON_AddStringToObject(json, "action", "LOGIN");
                    cJSON_AddStringToObject(json, "username", username);
                    cJSON_AddStringToObject(json, "password", "mypassword");
                    break;
                }
                default:
                    printf("Invalid choice\n");
                    cJSON_Delete(json);
                    continue;
            }

            // Convert the JSON object to a string
            char *json_str = cJSON_Print(json);

            // Send JSON data to the server
            send(sock, json_str, strlen(json_str), 0);
            printf("Sent: %s\n", json_str);

            // Clean up
            cJSON_Delete(json);
            free(json_str);
        }
    }

    // Clean up and close socket
    close(sock);
    return 0;
}
