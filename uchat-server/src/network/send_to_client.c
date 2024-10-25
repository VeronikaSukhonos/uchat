#include <uchat_server.h>

// Send a message to a specific client
bool send_message_to_client(Client clients[], const char *message, const char *receiver_username,
                            int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i].socket != 0 && strcmp(clients[i].username, receiver_username) == 0) {
            send(clients[i].socket, message, strlen(message), 0);
            printf("Sent message to %s\n", receiver_username);
            return true;
        }
    }
    printf("User %s not found or not connected\n", receiver_username);
    return false;
}
