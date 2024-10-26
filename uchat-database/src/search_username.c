#include <database.h>

int search_username(sqlite3 *db, const char *username) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT COUNT(*) FROM `users` WHERE username = ?;";
  int found = 0;

  // Prepare the SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  // Bind the username parameter to the SQL statement
  if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
    fprintf(stderr, "Failed to bind username: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  // Execute the query and check if a result exists
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    int count = sqlite3_column_int(stmt, 0);
    found = (count > 0) ? 1 : 0;
  }

  // Finalize the statement to avoid memory leaks
  sqlite3_finalize(stmt);

  return found;
}
