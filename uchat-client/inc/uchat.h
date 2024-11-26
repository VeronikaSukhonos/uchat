#pragma once

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <cJSON.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERS_IN_GROUP_COUNT 5
#define KEY_SIZE 32  // 256-bit key for AES-256
#define IV_SIZE 16   // 128-bit IV for AES
#define SALT_SIZE 16 // 128-bit salt for PBKDF2
#define CACHE_DIR "cache"
#define MESSAGE_FILE_FORMAT "%s_messages.enc" // Encrypted message file format
#define VOICE_FILE_FORMAT "%s_voice.enc"

// Fixed salt for all clients
static unsigned char salt[SALT_SIZE] = {0x3f, 0x6a, 0x99, 0x41, 0xc7, 0xa9,
                                        0x87, 0x6e, 0x32, 0xd5, 0x87, 0xa1,
                                        0xb2, 0x77, 0x3d, 0x4e};

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
extern char username[64];

typedef enum { TEXT,
               VOICE } ContentType;

typedef enum { NEW,
               MODIFIED,
               DELETED } MessageStatus;

typedef struct {
    int message_id;
    int chat_id;
    char sender[64];
    time_t date;
    ContentType content_type;
    MessageStatus status;
    char content[1024];
    char voice_path[256];
    int read; // New field to indicate read (1) or unread (0)
    GtkWidget *username_label;
    GtkWidget *button;
    GtkWidget *voice_message_button;
    GtkWidget *message_label;
    GtkWidget *time_label;
    GtkWidget *changed_label;
    GtkWidget *seen_label;
} MessageCache;

// Linked list node for MessageCache
typedef struct MessageNode {
    MessageCache *message;
    struct MessageNode *next;
} MessageNode;

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
    GtkWidget *name_surname;
    GtkWidget *student_group;
    GtkWidget *description;
    GtkWidget *status;
    GtkWidget *form;
    GtkWidget *message;
    GtkWidget *role_combo;
    GtkWidget *old_pw;
    GtkWidget *new_pw;
    GtkWidget *new_pw_again;
    GtkWidget *email;
    GtkWidget *subject_combo;
    GtkWidget *support_request;
} t_profile_data;

typedef struct s_chat_data {
    GtkWidget *button;
    GtkWidget *name;
    GtkWidget *last_time;
    GtkWidget *last_sender;
    GtkWidget *last_message;
    GtkWidget *unread;
    GtkWidget *box;
    gboolean is_mic_active;
    int id;
} t_chat_data;

typedef struct s_chat_node {
    t_chat_data chat;
    struct s_chat_node *next;
} t_chat_node;

typedef struct s_main_page_data {
    int sock;
    GtkWidget *menu_stack;
    int menu_opened;
    GtkWidget *menu_button_selected;
    GtkWidget *chats_box;
    t_chat_node *chats;
    MessageNode *messages;
    int chats_count;
    t_chat_data *opened_chat;
    GtkWidget *central_area_stack;
    t_chat_form_data create_chat_data;
    t_chat_form_data create_group_data;
    t_profile_data profile_data;
    t_profile_data edit_data;
    t_profile_data settings_data;
    t_profile_data change_pw;
    t_profile_data support;
    t_profile_data email_change;
    GtkWidget *group_box;
    int group_users_count;
    t_group_users_data group_users[USERS_IN_GROUP_COUNT];
    GtkWidget *chats_stack;
    GtkWidget *messages_container;
    GtkTextView *message_entry;
    GtkTextBuffer *message_buffer;
    GtkWidget *smile_window;
    GtkWidget *mic_button;
    GtkWidget *chat_nickname;  
    // Add fields for page switching
    const char *current_page;
    const char *previous_page;
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

typedef struct s_reconnect_message {
    GtkWidget *box;
    GtkWidget *label;
} t_reconnect_message;

typedef struct {
    GtkWidget *pages;
    GtkWidget *registration;
    GtkWidget *login;
    GtkWidget *chats;
    t_form_data *registration_data;
    t_form_data *login_data;
    t_main_page_data *main_page;
    GtkWidget *main_overlay;
    t_reconnect_message *reconnect;
} AppData;

// encrypt
int derive_key_from_serial(const char *serial, unsigned char *key);
int encrypt_session(const unsigned char *plaintext, int plaintext_len,
                    unsigned char *key, unsigned char *ciphertext,
                    unsigned char *iv);
int decrypt_session(const unsigned char *ciphertext, int ciphertext_len,
                    unsigned char *key, unsigned char *iv,
                    unsigned char *plaintext);

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
void play_voice(GtkWidget *button, gpointer data);
// response
int handle_get_profile_response(cJSON *response, AppData *app_data);

int handle_response(int sock, int *logged_in, AppData *app_data);
int handle_login_response(cJSON *response);

int connect_to_server(const char *server_ip, int port);
void get_serial_number(char *serial, size_t len);

void handle_not_logged_in_choice(int sock);
void handle_logged_in_choice(int sock, const char *username);
void process_voice_message_and_store(const char *json_response,
                                     AppData *app_data);

// gui
void load_css(const gchar *file);

void new_chat_button_from_json(t_main_page_data *main_page, int chat_id,
                               const char *name, char *chat_type,
                               const char *last_message,
                               const char *last_sender, const char *last_time,
                               const char *unread);
void create_registration_page(GtkWidget *pages, GtkWidget *registration,
                              t_form_data *data);
void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data);
void create_chats_page(GtkWidget *pages, GtkWidget *chats,
                       t_main_page_data *main_page);
void close_main_window(GtkWidget *main_window, t_main_page_data *main_page);
void show_smile_menu(GtkWidget *smile_button, GtkWidget *smile_window);
void hide_smile_menu(GtkWidget *smile_window, GtkWidget *smile_button);
void insert_emoji_into_text(GtkWidget *emoji_button,
                            t_main_page_data *main_page);
MessageNode *create_message_node(t_main_page_data *main_page,
                                 ContentType message_type, int chat_id,
                                 cJSON *message_json);
void create_message_button(t_main_page_data *main_page, MessageNode *temp_node);

void show_chat(GtkWidget *chat_button, gpointer data);
void show_registration(GtkWidget *registration_link_button, t_form_data *data);
void show_login(GtkWidget *login_link_button, t_form_data *data);
void show_profile(GtkWidget *settings_button, gpointer data);
void show_edit_page(GtkWidget *edit_button, gpointer data);
void show_new_group(GtkWidget *new_group_button, gpointer data);
void show_new_chat(GtkWidget *new_chat_button, gpointer data);
void change_entry_box_focus(GtkWidget *entry, GdkEventFocus *event,
                            GtkWidget *entry_box);
void change_password_visibility(GtkWidget *pw_button, GtkWidget *pw_entry);

void registration_submit(GtkWidget *registration_button, t_form_data *data);
void login_submit(GtkWidget *login_button, t_form_data *data);
void chat_creation(GtkWidget *create_chat_button, gpointer data);
void group_creation(GtkWidget *create_group_button, gpointer data);
void adding_user(GtkWidget *add_user_button, gpointer data);
void removing_user(GtkWidget *clicked_button, gpointer data);
void set_selected_button(GtkWidget **selected_button,
                         GtkWidget **new_selected_button);
void change_profile(GtkWidget *change_button, gpointer data);

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
                         t_form_data *login_data, t_main_page_data *main_page,
                         GtkWidget *main_overlay,
                         t_reconnect_message *reconnect);
int attempt_main_reconnection(AppData *app_data);
gboolean periodic_reconnection_attempt(gpointer data);
void create_reconnect_message(GtkWidget *main_overlay,
                              t_reconnect_message *reconnect);
void on_retry_clicked(GtkButton *button, gpointer data);

// input box gui
void check_message_entry_height(GtkTextBuffer *message_buffer,
                                GtkWidget *message_entry);
gboolean on_button_hover(GtkWidget *send_button, GdkEvent *event,
                         gpointer user_data);
gboolean on_button_leave(GtkWidget *send_button, GdkEvent *event,
                         gpointer user_data);
gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event,
                            gpointer user_data);
void change_button_hover_image(GtkWidget *send_button);
const gchar *message_trim(const gchar *message);
void send_message_to_server(int chat_id, const gchar *message);
void send_message_f(GtkWidget *widget, gpointer data);
void add_message(GtkWidget *messages_container, const gchar *message_text,
                 gboolean is_my_message);
void on_message_send(GtkWidget *send_button, gpointer user_data);
void on_message_received(const gchar *message_text);

// settings gui
void show_pw(GtkWidget *edit_button, gpointer data);
void change_password(GtkWidget *change_button, gpointer data);
void show_settings(GtkWidget *settings_button, gpointer data);
void show_support(GtkWidget *edit_button, gpointer data);
void send_support_request(GtkWidget *support_button, gpointer data);
void success_or_error_msg(GtkWidget *label, const char *message, gboolean is_success);
void show_email(GtkWidget *support_button, gpointer data);
gboolean check_email(const char *email, GtkWidget *message_label);
int check_username(char *username, GtkWidget *message);
int check_password(char *password, GtkWidget *message);
void change_email(GtkWidget *change_button, gpointer data);

// menu gui
void change_mic_image(GtkWidget *mic_button, gpointer data);
void open_close_menu(GtkWidget *menu_button, gpointer data);
void set_selected_button(GtkWidget **selected_button, GtkWidget **new_selected_button);
void log_out(GtkWidget *log_out_button, gpointer data);

// switch pages with delay
gboolean delayed_page_switch(gpointer data);
void switch_to_page_with_delay(t_main_page_data *main_page, const char *page_name, guint delay_ms);

// cache
MessageNode *load_encrypted_messages_from_cache(const char *chat_id);
int save_encrypted_messages_to_cache(const char *chat_id,
                                     MessageNode *messages);
void save_encrypted_chat_to_cache(const char *file_path, cJSON *chat_data);
MessageNode *append_message_node(MessageNode *head, MessageCache message);
void print_messages(MessageNode *head);
void free_message_list(MessageNode *head);
void ensure_cache_directory();

void stop_recording();
void start_recording(const char *output_path);
void send_voice_message(int sock, const char *file_path, int chat_id);

char *base64_encode(const unsigned char *data, size_t input_length);
char *read_and_encode_file(const char *filepath);
void save_decoded_file(const char *encoded_data, const char *output_file);
unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length);

void get_last_message_info(MessageNode *messages, char *sender, char *content,
                           time_t *date);
void handle_chat_list_response(cJSON *response, const char *cache_dir);
char *decrypt_json_from_file(const char *file_path);
int read_chat_data_from_encrypted_json(const char *file_path, int *chat_id,
                                       char *name, char *chat_type,
                                       char *last_message, char *last_sender,
                                       char *last_time, char *unread);
void create_chat_buttons_from_encrypted_cache(t_main_page_data *main_page,
                                              const char *cache_dir);
void save_single_chat_to_encrypted_cache(cJSON *chat, const char *cache_dir,
                                         AppData *app_data);
void delete_cache_directory();
void remove_all_chat_buttons(t_main_page_data *main_page);
int insert_message_into_chat(const char *file_path, cJSON *new_message);
void process_message_and_store(const char *json_response, AppData *app_data);
void create_or_update_chat_button(t_main_page_data *main_page, int chat_id,
                                  const char *name, const char *chat_type,
                                  const char *last_message,
                                  const char *last_sender,
                                  const char *last_time, const char *unread);
void create_msg_buttons_from_cache(t_main_page_data *main_page,
                                   const char *cache_dir);

void show_profile_from_icon(GtkWidget *profile_icon, gpointer data);
void show_participant_profile(GtkWidget *profile_icon, gpointer data);
