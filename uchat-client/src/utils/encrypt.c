#include <openssl/evp.h>
#include <openssl/rand.h>
#include <uchat.h>

// Derive an encryption key from the serial number and fixed salt
int derive_key_from_serial(const char *serial, unsigned char *key) {
  // Derive the key using PBKDF2
  if (PKCS5_PBKDF2_HMAC(serial, strlen(serial), salt, SALT_SIZE, 10000,
                        EVP_sha256(), KEY_SIZE, key) != 1) {
    fprintf(stderr, "Error deriving key.\n");
    return -1;
  }
  return 0; // Success
}

// Encrypt session data
int encrypt_session(const unsigned char *plaintext, int plaintext_len,
                    unsigned char *key, unsigned char *ciphertext,
                    unsigned char *iv) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  int len, ciphertext_len;

  if (!ctx)
    return -1;

  // Initialize encryption context with AES-256-CBC
  if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    return -1;

  // Encrypt the plaintext
  if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    return -1;
  ciphertext_len = len;

  if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    return -1;
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return ciphertext_len;
}

// Decrypt session data
int decrypt_session(const unsigned char *ciphertext, int ciphertext_len,
                    unsigned char *key, unsigned char *iv,
                    unsigned char *plaintext) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  int len, plaintext_len;

  if (!ctx)
    return -1;

  // Initialize decryption context with AES-256-CBC
  if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    return -1;

  if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    return -1;
  plaintext_len = len;

  if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    return -1;
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);
  return plaintext_len;
}
