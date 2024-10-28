#include <database.h>

int create_private_chat(sqlite3 *db, const char *chat_name) {
  sqlite3_stmt *stmt;
  const char *create_chat_sql =
      "INSERT INTO chats (name, type) VALUES (?, 'private');";

  if (sqlite3_prepare_v2(db, create_chat_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, chat_name, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to create chat: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  int chat_id = (int)sqlite3_last_insert_rowid(db);
  sqlite3_finalize(stmt);
  return chat_id;
}

int create_private_group_chat(sqlite3 *db, const char *chat_name) {
  sqlite3_stmt *stmt;
  const char *create_chat_sql =
      "INSERT INTO chats (name, type) VALUES (?, 'private_group');";

  if (sqlite3_prepare_v2(db, create_chat_sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return -1;
  }

  sqlite3_bind_text(stmt, 1, chat_name, -1, SQLITE_STATIC);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to create chat: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return -1;
  }

  int chat_id = (int)sqlite3_last_insert_rowid(db);
  sqlite3_finalize(stmt);
  return chat_id;
}