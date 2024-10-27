#include <database.h>

int add_chat_member(sqlite3 *db, int chat_id, int user_id) {
  sqlite3_stmt *stmt;
  const char *add_member_sql = "INSERT INTO chat_members (chat_id, user_id, "
                               "role) VALUES (?, ?, 'member');";

  if (sqlite3_prepare_v2(db, add_member_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for adding chat members: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }

  sqlite3_bind_int(stmt, 1, chat_id);
  sqlite3_bind_int(stmt, 2, user_id);

  int result = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : 1;
  if (result != 0) {
    fprintf(stderr, "Failed to add user %d to chat %d: %s\n", user_id, chat_id,
            sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return result;
}