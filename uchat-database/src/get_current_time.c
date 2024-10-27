#include <database.h>

int get_current_time(sqlite3 *db, char *current_time) {
  const char *current_time_query = "SELECT datetime('now')";
  sqlite3_stmt *time_stmt;

  if (sqlite3_prepare_v2(db, current_time_query, -1, &time_stmt, NULL) !=
      SQLITE_OK) {
    fprintf(stderr, "Failed to prepare time query statement: %s\n",
            sqlite3_errmsg(db));
    return 1;
  }

  int result = sqlite3_step(time_stmt);
  if (result == SQLITE_ROW) {
    strcpy(current_time, (const char *)sqlite3_column_text(time_stmt, 0));
    sqlite3_finalize(time_stmt);
    return 0; // Successfully retrieved current time
  } else {
    sqlite3_finalize(time_stmt);
    return 1; // Failed to retrieve current time
  }
}
