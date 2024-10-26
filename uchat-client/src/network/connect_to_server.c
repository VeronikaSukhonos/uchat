#include <uchat.h>

int connect_to_server(const char *server_ip, int port) {
  int sock;
  struct sockaddr_in serv_addr;

  // Create socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Socket creation error\n");
    return -1;
  }

  // Setup server address
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IP address from text to binary form
  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    printf("Invalid address/Address not supported\n");
    close(sock);
    return -1;
  }

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("Connection failed\n");
    close(sock);
    return -1;
  }

  return sock; // Return the socket descriptor on success
}
