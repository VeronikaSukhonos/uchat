#include <fcntl.h>
#include <signal.h>
#include <uchat_server.h>

int port;
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

void daemonize() {
  pid_t pid;

  // Fork the process
  pid = fork();
  if (pid < 0) {
    perror("Fork failed");
    exit(EXIT_FAILURE);
  }

  // If we got a good PID, exit the parent process
  if (pid > 0) {
    printf("%d\n", pid);
    exit(EXIT_SUCCESS);
  }

  // Create a new session
  if (setsid() < 0) {
    perror("setsid failed");
    exit(EXIT_FAILURE);
  }

  // Redirect standard I/O to /dev/null
  int dev_null = open("/dev/null", O_RDWR);
  if (dev_null == -1) {
    perror("Failed to open /dev/null");
    exit(EXIT_FAILURE);
  }
  dup2(dev_null, STDIN_FILENO);
  dup2(dev_null, STDOUT_FILENO);
  dup2(dev_null, STDERR_FILENO);
  close(dev_null);
}

void print_usage(const char *program_name) {
  fprintf(stderr, "Usage: %s <port>\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  // Check if the correct number of arguments is provided
  if (argc != 2) {
    print_usage(argv[0]);
  }

  // Parse the port number
  char *endptr;
  port = strtol(argv[1], &endptr, 10);

  // Check if the port is a valid integer
  if (*endptr != '\0' || port <= 0 || port > 65535) {
    fprintf(stderr, "Error: Invalid port number '%s'.\n", argv[1]);
    print_usage(argv[0]);
  }

  struct sockaddr_in address;
  fd_set readfds;

  signal(SIGINT, handle_sigint);

  daemonize();
  //  Initialize server
  server_fd = server_init(&address);
  printf("Server listening on port %d\n", port);

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
