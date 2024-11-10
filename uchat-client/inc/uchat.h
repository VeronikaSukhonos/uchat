#pragma once

#include <arpa/inet.h>
#include <ctype.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <cJSON.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERS_IN_GROUP_COUNT 5

extern int sock;
extern int logged_in;
extern char session_token[65];
extern int main_window;
extern GtkWidget *retry_window;
extern GtkWidget *retry_label;
extern guint retry_timeout;
extern guint retry_timeout_id;
extern guint main_retry_timeout;
extern guint reconnect_timer_id;
extern GIOChannel *gio_channel;

typedef struct s_chat_form_data {
  GtkWidget *form;
  GtkWidget *name;
  GtkWidget *username;
  GtkWidget *message;
} t_chat_form_data;

typedef struct s_group_users_data {
  char username[50];
  GtkWidget *button;
} t_group_users_data;

// profile_data
typedef struct s_profile_data {
  GtkWidget *username;
  GtkWidget *description;
  GtkWidget *status;
  GtkWidget *form;
  GtkWidget *message;
} t_profile_data;

typedef struct s_main_page_data {
  int sock;
  GtkWidget *menu_stack;
  int menu_opened;
  GtkWidget *menu_button_selected;
  GtkWidget *central_area_stack;
  GtkWidget *chats_stack;
  t_chat_form_data create_chat_data;
  t_chat_form_data create_group_data;
  // profile_data
  t_profile_data profile_data;
  // edit_data
  t_profile_data edit_data;
  GtkWidget *group_box;
  int group_users_count;
  t_group_users_data group_users[USERS_IN_GROUP_COUNT];
} t_main_page_data;

typedef struct s_form_data {
  int sock;
  GtkWidget *form;
  GtkWidget *username;
  GtkWidget *password;
  GtkWidget *repassword;
  GtkWidget *pw_button;
  GtkWidget *repw_button;
  GtkWidget *message;
} t_form_data;

typedef struct {
  GtkWidget *pages;
  GtkWidget *registration;
  GtkWidget *login;
  GtkWidget *chats;
  t_form_data *registration_data;
  t_form_data *login_data;
  t_main_page_data *main_page;
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

// response
int handle_response(int sock, int *logged_in, AppData *app_data);
int handle_login_response(cJSON *response);

int connect_to_server(const char *server_ip, int port);
void get_serial_number(char *serial, size_t len);

void handle_not_logged_in_choice(int sock);
void handle_logged_in_choice(int sock, const char *username);

// gui
void load_css(const gchar *file);
void create_registration_page(GtkWidget *pages, GtkWidget *registration,
                              t_form_data *data);
void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data);
void create_chats_page(GtkWidget *pages, GtkWidget *chats,
                       t_main_page_data *main_page);

void show_registration(GtkWidget *registration_link_button, t_form_data *data);
void show_login(GtkWidget *login_link_button, t_form_data *data);
void change_password_focus(GtkWidget *pw_entry, GdkEventFocus *event,
                           GtkWidget *pw_container);
void change_password_visibility(GtkWidget *pw_button, GtkWidget *pw_entry);

void registration_submit(GtkWidget *registration_button, t_form_data *data);
void login_submit(GtkWidget *login_button, t_form_data *data);
void removing_user(GtkWidget *clicked_button, gpointer data);

int check_form_data(char *username, char *password, GtkWidget *message);

int attempt_reconnection();
gboolean retry_connection(gpointer data);
void setup_main_application();
void create_update_failed_window();
gboolean on_server_data(GIOChannel *source, GIOCondition condition,
                        gpointer data);
void setup_gtk_interface(GtkWidget *pages, GtkWidget *registration,
                         GtkWidget *login, GtkWidget *chats,
                         t_form_data *registration_data,
                         t_form_data *login_data, t_main_page_data *main_page);
int attempt_main_reconnection(AppData *app_data);
gboolean periodic_reconnection_attempt(gpointer data);
void on_retry_clicked(GtkButton *button, gpointer data);
