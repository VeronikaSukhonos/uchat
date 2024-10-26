#include <database.h>

void update_last_seen(sqlite3 *db, int user_id, int is_online) {
  sqlite3_stmt *stmt;
  const char *sql =
      (is_online == 0)
          ? "UPDATE users SET last_seen = '1970-01-01 00:00:00' WHERE id = ?;"
          : "UPDATE users SET last_seen = CURRENT_TIMESTAMP WHERE id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return;
  }

  sqlite3_bind_int(stmt, 1, user_id);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to update last_seen: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
}
