#include <database.h>

int insert_user_ip(sqlite3 *db, int user_id, const char *ip_address,
                   const char *serial_number) {
  sqlite3_stmt *stmt;
  const char *check_sql = "SELECT 1 FROM user_ips WHERE user_id = ? AND "
                          "ip_address = ? AND serial_number = ?";

  // Prepare statement to check for an existing entry
  if (sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare check statement: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, ip_address, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, serial_number, -1, SQLITE_STATIC);

  int exists =
      sqlite3_step(stmt) == SQLITE_ROW; // If we get a row, the entry exists
  sqlite3_finalize(stmt);

  if (exists) {
    printf(
        "Debug: Entry with user_id %d, IP %s, and Serial %s already exists.\n",
        user_id, ip_address, serial_number);
    return 0; // Row already exists, so do nothing
  }

  // Insert the new entry if it doesn't already exist
  const char *insert_sql = "INSERT INTO user_ips (user_id, ip_address, "
                           "serial_number) VALUES (?, ?, ?)";
  if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare insert statement: %s\n",
            sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, ip_address, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, serial_number, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert user IP: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  sqlite3_finalize(stmt);
  printf("Debug: Successfully inserted new entry for user_id %d with IP %s and "
         "Serial %s.\n",
         user_id, ip_address, serial_number);
  return 0;
}
