#include <database.h>

// Helper function to store notification in the database
int store_notification(sqlite3 *db, int user_id, int message_id) {
  sqlite3_stmt *stmt;
  const char *sql =
      "INSERT INTO notifications (user_id, message_id, is_delivered) "
      "VALUES (?, ?, 0);"; // Initially set is_delivered to 0

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare notification insert statement: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_int(stmt, 2, message_id);

  int result = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
  sqlite3_finalize(stmt);
  return result;
}

// Helper function to store a message in the database
int store_message(sqlite3 *db, int chat_id, int sender_id,
                  const char *content) {
  sqlite3_stmt *stmt;
  const char *sql =
      "INSERT INTO messages (chat_id, sender_id, content, is_read, status) "
      "VALUES (?, ?, ?, 0, 'new');";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare message insert statement: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, sender_id);
  sqlite3_bind_text(stmt, 3, content, -1, SQLITE_STATIC);

  int message_id =
      (sqlite3_step(stmt) == SQLITE_DONE) ? sqlite3_last_insert_rowid(db) : -1;
  sqlite3_finalize(stmt);

  return message_id;
}
