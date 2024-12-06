#include <database.h>
#include <sqlite3.h>
#include <stdio.h>
#include <time.h>

// Function to store a file in the database
int store_file_message(sqlite3 *db, int chat_id, int sender_id,
                       const char *file_type, const unsigned char *file_data,
                       size_t file_size, char *file_path) {
  const char *sql_message = "INSERT INTO messages (chat_id, sender_id, type, "
                            "created_at, status) "
                            "VALUES (?, ?, 'file', ?, 'new');";
  const char *sql_file = "INSERT INTO files (message_id, file_path, file_type, "
                         "size, uploaded_at, file) "
                         "VALUES (?, ?, ?, ?, ?, ?);";
  sqlite3_stmt *stmt_message;
  sqlite3_stmt *stmt_file;
  int message_id = -1;

  // Step 1: Insert into the messages table
  if (sqlite3_prepare_v2(db, sql_message, -1, &stmt_message, NULL) !=
      SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for messages: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  // Bind parameters for the messages table
  sqlite3_bind_int(stmt_message, 1, chat_id);
  sqlite3_bind_int(stmt_message, 2, sender_id);

  // Get the current timestamp
  time_t now = time(NULL);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", gmtime(&now));
  sqlite3_bind_text(stmt_message, 3, timestamp, -1, SQLITE_STATIC);

  // Execute the statement for the messages table
  if (sqlite3_step(stmt_message) != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert file message into messages table: %s\n",
            sqlite3_errmsg(db));
    sqlite3_finalize(stmt_message);
    return -1;
  }

  // Retrieve the last inserted message ID
  message_id = (int)sqlite3_last_insert_rowid(db);
  sqlite3_finalize(stmt_message);

  // Step 2: Insert into the files table
  if (sqlite3_prepare_v2(db, sql_file, -1, &stmt_file, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for files: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  // Bind parameters for the files table
  sqlite3_bind_int(stmt_file, 1, message_id);
  sqlite3_bind_text(stmt_file, 2, file_path, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt_file, 3, file_type, -1, SQLITE_STATIC);
  sqlite3_bind_int(stmt_file, 4, (int)file_size);
  sqlite3_bind_text(stmt_file, 5, timestamp, -1, SQLITE_STATIC);
  sqlite3_bind_blob(stmt_file, 6, file_data, (int)file_size, SQLITE_STATIC);

  // Execute the statement for the files table
  if (sqlite3_step(stmt_file) != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert file into files table: %s\n",
            sqlite3_errmsg(db));
    sqlite3_finalize(stmt_file);
    return -1;
  }

  sqlite3_finalize(stmt_file);

  printf("File stored with associated message ID: %d\n", message_id);
  return message_id;
}