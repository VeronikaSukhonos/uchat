#include <uchat_server.h>

// Check for new connections and handle client activities
void check_client_activities(int server_fd, Client clients[], fd_set *readfds) {
  sqlite3 *db;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE];

  if (open_database(&db) != 0) {
    fprintf(stderr, "Failed to open database.\n");
  }
  delete_all_expired_sessions(db);
  sqlite3_close(db);

  // Handle new connection
  if (FD_ISSET(server_fd, readfds)) {
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("New connection from %s:%d\n", inet_ntoa(address.sin_addr),
           ntohs(address.sin_port));

    // Add new socket to the client array and store the IP address
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (clients[i].socket == 0) {
        clients[i].socket = new_socket;
        strcpy(clients[i].username, ""); // Initialize with an empty username
        strncpy(clients[i].ip_address, inet_ntoa(address.sin_addr),
                IP_LENGTH); // Store IP address
        memset(clients[i].serial_number, 0,
               FINGERPRINT_LENGTH); // Initialize empty fingerprint
        printf("Added client to socket list at index %d, IP: %s\n", i,
               clients[i].ip_address);
        break;
      }
    }
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

        if (open_database(&db) != 0) {
          fprintf(stderr, "Failed to open database.\n");
        }
        handle_disconnect(db, clients[i].username);
        sqlite3_close(db);
        close(sd);
        clients[i].socket = 0;
        strcpy(clients[i].username, "");             // Clear username
        memset(clients[i].ip_address, 0, IP_LENGTH); // Clear IP address
        memset(clients[i].serial_number, 0,
               FINGERPRINT_LENGTH); // Clear device fingerprint
      } else {
        // Handle client request
        buffer[valread] = '\0';

        // Check if the device fingerprint is already set
        if (clients[i].serial_number[0] == '\0') {
          // Parse JSON and capture device fingerprint on the first message
          // (e.g., during login)
          cJSON *json = cJSON_Parse(buffer);
          if (json != NULL) {
            cJSON *fingerprint = cJSON_GetObjectItem(json, "serial_number");
            if (fingerprint && cJSON_IsString(fingerprint)) {
              strncpy(clients[i].serial_number, fingerprint->valuestring,
                      FINGERPRINT_LENGTH);
              printf("Stored device fingerprint for client %d: %s\n", i,
                     clients[i].serial_number);
            }
            cJSON_Delete(json);
          }
        }

        handle_request(&clients[i], buffer, clients, MAX_CLIENTS, db);
      }
    }
  }
}