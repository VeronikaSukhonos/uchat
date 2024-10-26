#include <database.h>
#include <unistd.h>

int open_database(sqlite3 **db) {
  // Attempt to open the database
  if (sqlite3_open("uchat-database/UchatDB.db", db) != SQLITE_OK) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
    return 1;
  }
  return 0; // Successfully opened
}
