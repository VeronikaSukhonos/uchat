#include <uchat_server.h>

int handle_disconnect(sqlite3 *db, char *username) {
  int id = get_user_id(db, username);

  if (id == -1) {
    return 1;
  }

  update_last_seen(db, id, 1);

  return 0;
}
