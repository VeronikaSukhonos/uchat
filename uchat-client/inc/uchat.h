#pragma once

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <cJSON.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int save_session(const char *username, const char *session_token);
int load_session(char *username, size_t username_size, char *session_token,
                 size_t token_size);
int check_session_on_server(int sock, const char *username,
                            const char *session_token,
                            const char *serial_number);
int delete_session();

char *build_json_register(const char *username, const char *email,
                          const char *password);
char *build_json_login(const char *username, const char *password,
                       const char *serial_number);
char *build_json_message(const char *receiver, const char *message);
char *build_json_group_chat(const char *chat_name, char usernames[][50],
                            int num_users);
void handle_response(int sock, int *logged_in);

int connect_to_server(const char *server_ip, int port);
void get_serial_number(char *serial, size_t len);

void handle_not_logged_in_choice(int sock);
void handle_logged_in_choice(int sock, const char *username);