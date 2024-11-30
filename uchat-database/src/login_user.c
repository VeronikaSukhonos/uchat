#include <database.h>

int authenticate_user(sqlite3 *db, const char *username,
                      const char *password_hash) {
  sqlite3_stmt *stmt;
  const char *sql =
      "SELECT id FROM users WHERE username = ? AND password_hash = ?;";
  int user_id = -1;

  // Prepare SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  // Bind parameters
  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);

  // Execute and fetch result
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    user_id = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return user_id;
}

// Function to create a session and generate a unique token
int create_session(sqlite3 *db, int user_id, char *session_token,
                   int token_size) {
  sqlite3_stmt *stmt;
  // delete_user_tokens(db, user_id);
  const char *sql =
      "INSERT INTO sessions (user_id, token, created_at, expires_at) VALUES "
      "(?, ?, CURRENT_TIMESTAMP, datetime(CURRENT_TIMESTAMP, '+24 hour'));";

  // Generate a random session token (use secure generation in production)
  generate_insecure_token(session_token, token_size);

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, session_token, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to create session: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  sqlite3_finalize(stmt);
  return 0;
}

// Function to log in a user, set as online, and create a session
int login_user(sqlite3 *db, const char *username, const char *serial_number,
               const char *ip_address, const char *password_hash,
               char *session_token, int token_size) {
  int user_id = authenticate_user(db, username, password_hash);
  if (user_id == -1) {
    fprintf(stderr, "Authentication failed.\n");
    return -1;
  }

  // Set user as online
  update_last_seen(db, user_id, 0);

  // Create a session and populate session_token
  if (create_session(db, user_id, session_token, token_size) != 0) {
    fprintf(stderr, "Failed to create session.\n");
    return -1;
  }
  if (insert_user_ip(db, user_id, ip_address, serial_number) != 0) {
    fprintf(stderr, "Failed to store user IP and serial number.\n");
    return -1;
  }

  printf("User '%s' logged in successfully.\n", username);
  return 0;
}