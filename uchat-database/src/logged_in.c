#include <database.h>

int logged_in(sqlite3 *db, int user_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT last_seen FROM users WHERE id = ?;";
  int rc;
  int is_logged_in = 0; // Default to not logged in

  // Prepare the SQL statement
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1; // Error in preparing statement
  }

  // Bind the user_id parameter to the SQL statement
  rc = sqlite3_bind_int(stmt, 1, user_id);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to bind user_id: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1; // Error in binding parameter
  }

  // Execute the query and check if last_seen is 1970-01-01 00:00:00
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char *last_seen = sqlite3_column_text(stmt, 0);
    if (last_seen &&
        strcmp((const char *)last_seen, "1970-01-01 00:00:00") == 0) {
      is_logged_in = 1; // User is logged in
    }
  } else {
    fprintf(stderr, "User ID %d not found.\n", user_id);
  }

  // Finalize the statement to release resources
  sqlite3_finalize(stmt);

  return is_logged_in;
}
