#include <uchat_server.h>

// Helper function to check if a private chat already exists between two users
int check_existing_chat(sqlite3 *db, int user_id, int other_user_id) {
  sqlite3_stmt *stmt;
  const char *check_chat_sql =
      "SELECT id FROM chats WHERE type = 'private' AND id IN "
      "(SELECT chat_id FROM chat_members WHERE user_id = ?) "
      "AND id IN (SELECT chat_id FROM chat_members WHERE user_id = ?);";

  if (sqlite3_prepare_v2(db, check_chat_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_int(stmt, 2, other_user_id);

  int chat_exists = (sqlite3_step(stmt) == SQLITE_ROW);
  sqlite3_finalize(stmt);

  return chat_exists;
}
