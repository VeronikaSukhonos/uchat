#include <database.h>

int get_user_id(sqlite3 *db, const char *username) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT id FROM users WHERE username = ?;";
  int user_id = -1;

  // Prepare SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  // Bind parameters
  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

  // Execute and fetch result
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    user_id = sqlite3_column_int(stmt, 0);
  }

  sqlite3_finalize(stmt);
  return user_id;
}