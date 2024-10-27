#include <database.h>

int register_user(sqlite3 *db, const char *username, const char *gmail,
                  const char *password_hash) {
  sqlite3_stmt *stmt;
  const char *sql =
      "INSERT INTO users (username, gmail, password_hash) VALUES (?, ?, ?);";
  int rc;

  // Prepare the SQL statement
  if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  // Bind the parameters to the SQL statement
  sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, gmail, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, password_hash, -1, SQLITE_STATIC);

  // Execute the SQL statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert user: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return rc;
  }

  // Finalize the statement to release resources
  sqlite3_finalize(stmt);

  printf("User '%s' registered successfully.\n", username);
  return SQLITE_OK;
}
