#include <uchat_server.h>

void generate_insecure_token(char *session_token, int token_size) {
  srand((unsigned int)time(NULL));

  for (int i = 0; i < token_size - 1; ++i) {
    int random_value = rand() % 62;
    if (random_value < 26) {
      session_token[i] = 'A' + random_value;
    } else if (random_value < 52) {
      session_token[i] = 'a' + (random_value - 26);
    } else {
      session_token[i] = '0' + (random_value - 52);
    }
  }
  session_token[token_size - 1] = '\0';
}