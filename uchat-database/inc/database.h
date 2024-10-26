#pragma once

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int open_database(sqlite3 **db);
int search_username(sqlite3 *db, const char *username);
int register_user(sqlite3 *db, const char *username, const char *gmail,
                  const char *password_hash);
int login_user(sqlite3 *db, const char *username, const char *serial_number,
               const char *ip_address, const char *password_hash,
               char *session_token, int token_size);
int get_user_id(sqlite3 *db, const char *username);
int delete_user_tokens(sqlite3 *db, int user_id);
void update_last_seen(sqlite3 *db, int user_id, int is_online);
int logged_in(sqlite3 *db, int user_id);

int insert_user_ip(sqlite3 *db, int user_id, const char *ip_address,
                   const char *serial_number);