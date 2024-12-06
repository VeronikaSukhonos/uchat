#pragma once

#include <arpa/inet.h>
#include <cJSON.h>
#include <database.h>
#include <errno.h>
#include <netinet/in.h>
#include <sha256.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#define SHA256_HASH_SIZE (SHA256_SIZE_BYTES * 2 + 1)
#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 30 // Max number of clients to handle
#define IP_LENGTH 16   // IPv4 address length
#define FINGERPRINT_LENGTH 256

typedef struct {
  int socket;
  char username[50];
  char ip_address[IP_LENGTH];             // Store the client's IP address
  char serial_number[FINGERPRINT_LENGTH]; // Store the username for the
  char in_call_with[50];
} Client;

// network
int server_init(struct sockaddr_in *address);
void check_client_activities(int server_fd, Client clients[], fd_set *readfds);

bool send_message_to_client(Client clients[], const char *message,
                            const char *receiver_username, int max_clients);
void send_status_responce_to_client(Client *client, const char *action,
                                    const char *status);
void send_json_responce_to_client(Client *client, cJSON *json);
void push_message_to_all_clients(Client clients[], const char *message,
                                 int max_clients);

// requests
int handle_get_profile(sqlite3 *db, Client *client);
void handle_request(Client *client, char *buffer, Client clients[],
                    int max_clients, sqlite3 *db);
int handle_register(sqlite3 *db, cJSON *json);
int handle_login(sqlite3 *db, cJSON *json, char *session_token, Client *client);
int handle_logout(sqlite3 *db, char *username);
int handle_disconnect(sqlite3 *db, char *username);
int handle_create_chat(sqlite3 *db, cJSON *json, Client *client,
                       Client clients[], int max_clients);
int handle_create_group_chat(sqlite3 *db, cJSON *json, Client *client,
                             Client clients[], int max_clients);
int handle_get_chat_list(sqlite3 *db, Client *client);
int handle_send_message_to_chat(sqlite3 *db, Client *client, cJSON *json,
                                Client clients[], int max_clients);
int handle_find_user(sqlite3 *db, cJSON *req, Client *client);
int handle_update_profile(sqlite3 *db, Client *client, cJSON *json);
int handle_voice_message_to_chat(sqlite3 *db, Client *client, cJSON *json,
                                 Client clients[], int max_clients);

// utils
void hash_password(const char *password, char *outputBuffer);
cJSON *build_json_login(const char *username, const char *token);
int handle_check_session(sqlite3 *db, cJSON *json, char *session_token,
                         Client *client);

void save_decoded_file(const char *encoded_data, const char *output_file);
unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length);
void handle_call_forward(Client clients[], Client *client, cJSON *json,
                         int max_clients);
void handle_accept_call_forward(Client clients[], Client *client, cJSON *json,
                                int max_clients);
void handle_stop_call_forward(Client clients[], Client *client, cJSON *json,
                              int max_clients);
void handle_get_new_data_request(sqlite3 *db, Client *client,
                                 cJSON *client_request);
int handle_update_message(sqlite3 *db, Client *client, cJSON *json,
                          Client clients[], int max_clients);
int handle_delete_message(sqlite3 *db, Client *client, cJSON *json,
                          Client clients[], int max_clients);
int handle_update_password(sqlite3 *db, Client *client, cJSON *json);
int handle_support_request(Client *client, cJSON *json);
int handle_get_settings(sqlite3 *db, Client *client);
int handle_file_message_to_chat(sqlite3 *db, Client *client, cJSON *json,
                                Client clients[], int max_clients);
