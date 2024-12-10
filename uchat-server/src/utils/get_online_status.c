#include <uchat_server.h>

int get_online_status(char *username, Client clients[], int max_clients) {
  for (int i = 0; i < max_clients; i++) {
    if (strcmp(clients[i].username, username) == 0) {
      return 1;
    }
  }
  return 0;
}