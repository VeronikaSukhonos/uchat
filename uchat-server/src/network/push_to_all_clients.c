#include <uchat_server.h>

// Broadcast a message to all clients
void push_message_to_all_clients(Client clients[], const char *message, int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        if (clients[i].socket != 0) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}
