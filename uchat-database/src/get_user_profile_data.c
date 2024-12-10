#include <database.h>

cJSON *get_user_profile_data(sqlite3 *db, int user_id) {

  // Updated query to fetch full_name, group, and role along with username
  char *query =
      "SELECT username, full_name, `group`, role FROM users WHERE id = ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, user_id);

  int result = sqlite3_step(stmt);
  if (result != SQLITE_ROW) {
    fprintf(stderr, "Failed to retrieve user data: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return NULL;
  }

  // Retrieve values from the query result
  const char *username = (const char *)sqlite3_column_text(stmt, 0);
  const char *full_name = (const char *)sqlite3_column_text(stmt, 1);
  const char *group = (const char *)sqlite3_column_text(stmt, 2);
  const char *role = (const char *)sqlite3_column_text(stmt, 3);

  // Create a JSON response with the retrieved values
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "full_name", full_name ? full_name : "");
  cJSON_AddStringToObject(response, "group", group ? group : "");
  cJSON_AddStringToObject(response, "role", role ? role : "");

  // Cleanup
  sqlite3_finalize(stmt);

  return response;
}