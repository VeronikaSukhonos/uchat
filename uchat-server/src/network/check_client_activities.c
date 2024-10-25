#include <uchat_server.h>

// Check for new connections and handle client activities
void check_client_activities(int server_fd, Client clients[], fd_set *readfds) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Handle new connection
    if (FD_ISSET(server_fd, readfds)) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) <
            0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("New connection from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Add new socket to the client array
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == 0) {
                clients[i].socket = new_socket;
                strcpy(clients[i].username, "");  // Initialize with an empty username
                printf("Added client to socket list at index %d\n", i);
                break;
            }
        }

        // push_message_to_all_clients(clients, "Hello everyone, we have a new client!",
        // MAX_CLIENTS);
    }

    // Handle data from clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = clients[i].socket;
        if (FD_ISSET(sd, readfds)) {
            int valread = read(sd, buffer, BUFFER_SIZE);

            // Client disconnected
            if (valread == 0) {
                getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                printf("Host disconnected, %s:%d\n", inet_ntoa(address.sin_addr),
                       ntohs(address.sin_port));
                close(sd);
                clients[i].socket = 0;
                strcpy(clients[i].username, "");  // Clear username
            } else {
                // Handle client request
                buffer[valread] = '\0';
                handle_request(&clients[i], buffer, clients, MAX_CLIENTS);
            }
        }
    }
}
