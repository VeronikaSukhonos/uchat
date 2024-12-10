#include <database.h>

void get_chat_type(sqlite3 *db, int chat_id, char *type) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT type FROM chats "
                    "WHERE id = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for chat type: %s\n",
            sqlite3_errmsg(db));
    return;
  }

  sqlite3_bind_int(stmt, 1, chat_id);

  int result = sqlite3_step(stmt);
  if (result == SQLITE_ROW) {
    strcpy(type, (char *)sqlite3_column_text(stmt, 0));
    sqlite3_finalize(stmt);
    return; // Successfully retrieved session details
  } else {
    sqlite3_finalize(stmt);
    return; // No matching session found
  }
}