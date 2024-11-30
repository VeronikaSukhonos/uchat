#pragma once

#include <cJSON.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int open_database(sqlite3 **db);
int search_username(sqlite3 *db, const char *username);
int register_user(sqlite3 *db, const char *username, const char *password_hash);
int login_user(sqlite3 *db, const char *username, const char *serial_number,
               const char *ip_address, const char *password_hash,
               char *session_token, int token_size);
int get_user_id(sqlite3 *db, const char *username);
int delete_user_tokens(sqlite3 *db, int user_id);
void update_last_seen(sqlite3 *db, int user_id, int is_online);
int logged_in(sqlite3 *db, int user_id);

int insert_user_ip(sqlite3 *db, int user_id, const char *ip_address,
                   const char *serial_number);

void generate_insecure_token(char *session_token, int token_size);
int get_current_time(sqlite3 *db, char *current_time);
int delete_expired_session(sqlite3 *db, int session_id);
int get_session_details(sqlite3 *db, int user_id, const char *session_token,
                        const char *ip_address, char *db_ip_address,
                        char *db_serial_number, char *expires_at,
                        int *session_id);
int check_existing_chat(sqlite3 *db, int user_id, int other_user_id);
int create_private_chat(sqlite3 *db, const char *chat_name);
int add_chat_member(sqlite3 *db, int chat_id, int user_id);
cJSON *retrieve_chat_list(sqlite3 *db, int user_id);
int store_message(sqlite3 *db, int chat_id, int sender_id, const char *content);
cJSON *get_online_chat_members(sqlite3 *db, int chat_id);
int store_notification(sqlite3 *db, int user_id, int message_id);
int create_private_group_chat(sqlite3 *db, const char *chat_name);
cJSON *get_chat_members(sqlite3 *db, int chat_id);
int delete_all_expired_sessions(sqlite3 *db);
cJSON *retrieve_last_20_messages(sqlite3 *db, int chat_id);
cJSON *get_message_details(sqlite3 *db, int message_id);
int store_voice_message(sqlite3 *db, int chat_id, int sender_id,
                        const unsigned char *voice_data, size_t voice_size);
char *base64_encode(const unsigned char *data, size_t input_length);
cJSON *retrieve_undelivered_messages(sqlite3 *db, int user_id, int chat_id);
cJSON *retrieve_chat_members(sqlite3 *db, int chat_id);