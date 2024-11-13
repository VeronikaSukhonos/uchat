#include <database.h>

int delete_all_expired_sessions(sqlite3 *db) {
  // SQL query to delete expired sessions based on their expiration date
  const char *delete_sql =
      "DELETE FROM sessions WHERE expires_at <= datetime('now')";
  sqlite3_stmt *delete_stmt;

  // Prepare the SQL statement
  if (sqlite3_prepare_v2(db, delete_sql, -1, &delete_stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare delete statement: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }

  // Execute the delete statement
  int result = sqlite3_step(delete_stmt);
  sqlite3_finalize(delete_stmt);

  // Check if the deletion was successful
  if (result == SQLITE_DONE) {
    printf("Debug: All expired sessions deleted successfully.\n");
    return 0;
  } else {
    fprintf(stderr, "Failed to delete expired sessions: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }
}