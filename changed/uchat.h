#pragma once

#include <arpa/inet.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <cJSON.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct s_form_data {
  int sock;
  GtkWidget *form;
  GtkWidget *username;
  GtkWidget *password;
  GtkWidget *repassword;
  GtkWidget *message;
} t_form_data;

typedef struct s_chat_form_data {
  GtkWidget *form;
  GtkWidget *name;
  GtkWidget *username;
  GtkWidget *message;
} t_chat_form_data;

typedef struct s_main_page_data {
    GtkWidget *menu_stack;
    int menu_opened;
    GtkWidget *central_area_stack;
    t_chat_form_data create_chat_data;
    t_chat_form_data create_group_data;
}       t_main_page_data;

typedef struct {
  GtkWidget *pages;
  GtkWidget *registration;
  GtkWidget *login;
  GtkWidget *chats;
  t_form_data *registration_data;
  t_form_data *login_data;
} AppData;

int save_session(const char *username, const char *session_token);
int load_session(char *username, size_t username_size, char *session_token,
                 size_t token_size);
int check_session_on_server(int sock, const char *username,
                            const char *session_token,
                            const char *serial_number);
int delete_session();

char *build_json_register(const char *username, const char *password);
char *build_json_login(const char *username, const char *password,
                       const char *serial_number);
char *build_json_message(const char *receiver, const char *message);
char *build_json_group_chat(const char *chat_name, char usernames[][50],
                            int num_users);
int handle_response(int sock, int *logged_in, AppData *app_data);

int connect_to_server(const char *server_ip, int port);
void get_serial_number(char *serial, size_t len);

void handle_not_logged_in_choice(int sock);
void handle_logged_in_choice(int sock, const char *username);

// gui
void load_css(const gchar *file);
void create_registration_page(GtkWidget *pages, GtkWidget *registration,
                              t_form_data *data);
void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data);
void create_chats_page(GtkWidget *pages, GtkWidget *chats, t_main_page_data *main_page);

void show_registration(GtkWidget *registration_link_button, gpointer pages);
void show_login(GtkWidget *login_link_button, gpointer pages);

void registration_submit(GtkWidget *registration_button, gpointer data);
void login_submit(GtkWidget *login_button, gpointer data);

int check_form_data(char *username, char *password, GtkWidget *message);