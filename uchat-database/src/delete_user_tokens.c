#include <database.h>

int delete_user_tokens(sqlite3 *db, int user_id) {
  sqlite3_stmt *stmt;
  const char *sql = "DELETE FROM sessions WHERE user_id = ?;";
  int rc;

  // Prepare the SQL statement
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  // Bind the user_id parameter to the SQL statement
  rc = sqlite3_bind_int(stmt, 1, user_id);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to bind user_id: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  // Execute the SQL statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to delete tokens: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  // Finalize the statement to release resources
  sqlite3_finalize(stmt);

  printf("All tokens for user ID %d have been deleted.\n", user_id);
  return 0; // Success
}