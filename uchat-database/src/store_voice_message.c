#include <database.h>
#include <sqlite3.h>
#include <stdio.h>
#include <time.h>

// Function to store a voice message in the database
int store_voice_message(sqlite3 *db, int chat_id, int sender_id,
                        const unsigned char *voice_data, size_t voice_size) {
  const char *sql = "INSERT INTO messages (chat_id, sender_id, type, "
                    "created_at, voice_message, status) "
                    "VALUES (?, ?, 'voice', ?, ?, 'new');";
  sqlite3_stmt *stmt;
  int message_id = -1;

  // Prepare the SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  // Bind parameters
  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, sender_id);

  // Get the current timestamp
  time_t now = time(NULL);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", gmtime(&now));
  sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_STATIC);

  // Bind the voice data as a BLOB
  sqlite3_bind_blob(stmt, 4, voice_data, (int)voice_size, SQLITE_STATIC);

  // Execute the statement
  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert voice message: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  // Retrieve the last inserted message ID
  message_id = (int)sqlite3_last_insert_rowid(db);

  // Finalize the statement
  sqlite3_finalize(stmt);

  printf("Voice message stored with ID: %d\n", message_id);
  return message_id;
}
