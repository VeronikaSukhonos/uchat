#pragma once

#include <arpa/inet.h>
#include <cJSON.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30  // Max number of clients to handle

typedef struct {
    int socket;
    char username[50];  // Store the username for the client
} Client;

// Function prototypes
int server_init(struct sockaddr_in *address);
void check_client_activities(int server_fd, Client clients[], fd_set *readfds);
void handle_request(Client *client, char *buffer, Client clients[], int max_clients);
bool send_message_to_client(Client clients[], const char *message, const char *receiver_username,
                            int max_clients);
void push_message_to_all_clients(Client clients[], const char *message, int max_clients);