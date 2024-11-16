#include <uchat_server.h>

#define INITIAL_BUFFER_SIZE                                                    \
  4096 // Adjust as necessary for expected response size

char *receive_large_json(int socket_fd) {
  size_t buffer_size = INITIAL_BUFFER_SIZE;
  char *buffer = malloc(buffer_size);
  if (!buffer) {
    perror("malloc failed");
    return NULL;
  }

  size_t received = 0;
  ssize_t bytes;
  int end_detected = 0;

  while (!end_detected) {
    // Blocking recv() call to read data
    bytes = recv(socket_fd, buffer + received, buffer_size - received, 0);

    if (bytes < 0) {
      perror("recv failed");
      free(buffer);
      return NULL;
    } else if (bytes == 0) {
      // Connection closed by the client
      fprintf(stderr, "Client disconnected.\n");
      free(buffer);
      return NULL; // Explicitly indicate disconnection
    }

    received += bytes;

    // If buffer is full, resize it
    if (received >= buffer_size) {
      buffer_size *= 2; // Double the buffer size
      char *temp = realloc(buffer, buffer_size);
      if (!temp) {
        perror("realloc failed");
        free(buffer);
        return NULL;
      }
      buffer = temp;
    }

    // Check if we've reached the end of a JSON object
    if (received >= 2 && buffer[received - 1] == '}' &&
        buffer[received - 2] != '\\') {
      end_detected = 1;
    }
  }

  buffer[received] = '\0'; // Null-terminate the JSON data
  return buffer;
}

// Check for new connections and handle client activities
void check_client_activities(int server_fd, Client clients[], fd_set *readfds) {
  sqlite3 *db;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

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
      // Use receive_large_json to dynamically handle incoming JSON
      char *json_data = receive_large_json(sd);

      // Client disconnected or error occurred
      if (!json_data) {
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
        continue;
      }

      // Check if the device fingerprint is already set
      if (clients[i].serial_number[0] == '\0') {
        // Parse JSON and capture device fingerprint on the first message
        cJSON *json = cJSON_Parse(json_data);
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

      // Handle client request
      handle_request(&clients[i], json_data, clients, MAX_CLIENTS, db);
    }
  }
}