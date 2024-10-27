#include <database.h>

int delete_expired_session(sqlite3 *db, int session_id) {
  const char *delete_sql = "DELETE FROM sessions WHERE id = ?";
  sqlite3_stmt *delete_stmt;

  if (sqlite3_prepare_v2(db, delete_sql, -1, &delete_stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare delete statement: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }

  sqlite3_bind_int(delete_stmt, 1, session_id);
  int result = sqlite3_step(delete_stmt);
  sqlite3_finalize(delete_stmt);

  if (result == SQLITE_DONE) {
    printf("Debug: Expired session deleted successfully.\n");
    return 0;
  } else {
    fprintf(stderr, "Failed to delete expired session: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }
}