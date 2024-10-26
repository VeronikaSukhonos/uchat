#include <uchat_server.h>

void hash_password(const char *password, char *outputBuffer) {
  sha256_context ctx;
  uint8_t hash[SHA256_SIZE_BYTES];

  // Initialize, update, and finalize the hash context
  sha256_init(&ctx);
  sha256_hash(&ctx, password, strlen(password));
  sha256_done(&ctx, hash);

  // Convert hash to hexadecimal string
  for (int i = 0; i < SHA256_SIZE_BYTES; i++) {
    sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }
  outputBuffer[SHA256_HASH_SIZE - 1] = '\0'; // Null-terminate the string
}