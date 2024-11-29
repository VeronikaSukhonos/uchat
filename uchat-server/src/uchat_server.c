#include <signal.h>
#include <uchat_server.h>
int server_fd;
Client clients[MAX_CLIENTS] = {0}; // Array of clients
void handle_sigint(int sig) {
  printf("\nCaught signal %d (Ctrl+C). Cleaning up and exiting...\n", sig);

  // Close all client sockets
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i].socket > 0) {
      close(clients[i].socket);
      clients[i].socket = 0;
    }
  }

  // Close the server socket
  close(server_fd);

  exit(0); // Exit the program
}

int main() {

  struct sockaddr_in address;
  fd_set readfds;

  signal(SIGINT, handle_sigint);
  // Initialize server
  server_fd = server_init(&address);
  printf("Server listening on port %d\n", PORT);

  while (1) {
    // Clear the socket set and add the server socket
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    int max_sd = server_fd;

    // Add client sockets to the set
    for (int i = 0; i < MAX_CLIENTS; i++) {
      int sd = clients[i].socket;
      if (sd > 0)
        FD_SET(sd, &readfds);
      if (sd > max_sd)
        max_sd = sd;
    }

    // Wait for activity on one of the sockets
    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) {
      perror("Select error");
    }

    // Check client activities
    check_client_activities(server_fd, clients, &readfds);
  }

  close(server_fd);
  return 0;
}
