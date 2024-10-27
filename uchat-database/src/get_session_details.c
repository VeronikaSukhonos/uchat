#include <database.h>

int get_session_details(sqlite3 *db, int user_id, const char *session_token,
                        const char *ip_address, char *db_ip_address,
                        char *db_serial_number, char *expires_at,
                        int *session_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT u.ip_address, u.serial_number, s.expires_at, s.id "
                    "FROM user_ips AS u "
                    "JOIN sessions AS s ON s.user_id = u.user_id "
                    "WHERE u.user_id = ? AND s.token = ? AND u.ip_address = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  sqlite3_bind_int(stmt, 1, user_id);
  sqlite3_bind_text(stmt, 2, session_token, -1, SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, ip_address, -1, SQLITE_STATIC);

  int result = sqlite3_step(stmt);
  if (result == SQLITE_ROW) {
    strcpy(db_ip_address, (const char *)sqlite3_column_text(stmt, 0));
    strcpy(db_serial_number, (const char *)sqlite3_column_text(stmt, 1));
    strcpy(expires_at, (const char *)sqlite3_column_text(stmt, 2));
    *session_id = sqlite3_column_int(stmt, 3);
    sqlite3_finalize(stmt);
    return 0; // Successfully retrieved session details
  } else {
    sqlite3_finalize(stmt);
    return 1; // No matching session found
  }
}
