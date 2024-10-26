#include <uchat_server.h>

int handle_logout(sqlite3 *db, char *username) {
  int id = get_user_id(db, username);

  if (id == -1) {
    return 1;
  }

  delete_user_tokens(db, id);
  update_last_seen(db, id, 1);

  return 0;
}
