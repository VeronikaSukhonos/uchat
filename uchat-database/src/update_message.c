#include <database.h>

int update_message(sqlite3 *db, int message_id, const char *new_content) {
  const char *sql = "UPDATE messages SET content = ?, status = ? WHERE id = ?;";
  sqlite3_stmt *stmt;

  // Prepare the SQL statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare update statement: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  // Bind values to the placeholders
  sqlite3_bind_text(stmt, 1, new_content, -1,
                    SQLITE_STATIC); // Bind new_content to ?
  sqlite3_bind_text(stmt, 2, "modified", -1,
                    SQLITE_STATIC);      // Bind new_status to ?
  sqlite3_bind_int(stmt, 3, message_id); // Bind message_id to ?

  // Execute the statement
  int result = sqlite3_step(stmt);
  if (result != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute update statement: %s\n",
            sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  // Clean up
  sqlite3_finalize(stmt);
  printf("Updated message ID %d successfully.\n", message_id);
  return 0;
}