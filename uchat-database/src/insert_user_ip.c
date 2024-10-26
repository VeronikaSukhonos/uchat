#include <database.h>

int insert_user_ip(sqlite3 *db, int user_id, const char *ip_address,
                   const char *serial_number) {
  const char *sql = "INSERT INTO user_ips (user_id, "
                    "ip_address, serial_number) VALUES (?, ?, ?)";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, ip_address, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, serial_number, -1, SQLITE_STATIC);

  int result = sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  if (result != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert user IP: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  return 0;
}
