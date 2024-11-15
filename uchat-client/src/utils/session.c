#include <openssl/evp.h>
#include <openssl/rand.h>
#include <uchat.h>

int check_session_on_server(int sock, const char *username,
                            const char *session_token,
                            const char *serial_number) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "CHECK_SESSION");
  cJSON_AddStringToObject(json, "username", username);
  cJSON_AddStringToObject(json, "serial_number", serial_number);
  cJSON_AddStringToObject(json, "session_token", session_token);
  char *json_str = cJSON_Print(json);

  // Send JSON data to the server
  send(sock, json_str, strlen(json_str), 0);
  char buffer[BUFFER_SIZE] = {0};

  int valread = read(sock, buffer, sizeof(buffer));
  if (valread > 0) {
    buffer[valread] = '\0';
    g_print("Server response: %s\n", buffer);
    cJSON *response = cJSON_Parse(buffer);
    if (response == NULL) {
      g_print("Failed to parse server response.\n");
      return 0;
    }

    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (strcmp(status->valuestring, "SUCCESS") == 0) {
      g_print("Session valid.\n");
      cJSON *chat_list_request = cJSON_CreateObject();
      cJSON_AddStringToObject(chat_list_request, "action", "GET_CHAT_LIST");
      char *chat_list_request_str = cJSON_Print(chat_list_request);
      send(sock, chat_list_request_str, strlen(chat_list_request_str), 0);
      g_print("Sent: %s\n", chat_list_request_str);
      free(chat_list_request_str);
      cJSON_Delete(chat_list_request);
      return 1;
    }
  } else if (valread == 0) {
    g_print("Server disconnected\n");
  }
  return 0;
}

// Save encrypted session to a file
int save_session(const char *username, const char *session_token) {
  unsigned char key[KEY_SIZE];
  char serial[128];
  get_serial_number(serial, sizeof(serial));

  if (derive_key_from_serial(serial, key) != 0)
    return -1;

  unsigned char iv[IV_SIZE];
  if (RAND_bytes(iv, IV_SIZE) != 1) {
    fprintf(stderr, "Failed to generate IV.\n");
    return -1;
  }

  // Prepare plaintext with username and session token
  unsigned char plaintext[512];
  snprintf((char *)plaintext, sizeof(plaintext), "%s\n%s\n", username,
           session_token);

  // Encrypt the plaintext
  unsigned char ciphertext[512];
  int ciphertext_len = encrypt_session(plaintext, strlen((char *)plaintext),
                                       key, ciphertext, iv);
  if (ciphertext_len < 0) {
    fprintf(stderr, "Encryption failed.\n");
    return -1;
  }

  // Write IV and ciphertext to file
  FILE *file = fopen("session.txt", "wb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open session file for writing.\n");
    return -1;
  }
  fwrite(iv, 1, IV_SIZE, file);                // Write IV
  fwrite(ciphertext, 1, ciphertext_len, file); // Write encrypted session data
  fclose(file);

  return 0;
}

// Load and decrypt session from a file
int load_session(char *username, size_t username_size, char *session_token,
                 size_t token_size) {
  unsigned char key[KEY_SIZE];
  char serial[128];
  get_serial_number(serial, sizeof(serial));

  if (derive_key_from_serial(serial, key) != 0)
    return -1;

  unsigned char iv[IV_SIZE];

  FILE *file = fopen("session.txt", "rb");
  if (file == NULL) {
    fprintf(stderr, "No session file found. Please log in.\n");
    return -1;
  }

  // Read IV and ciphertext from the file
  fread(iv, 1, IV_SIZE, file);
  unsigned char ciphertext[512];
  int ciphertext_len = fread(ciphertext, 1, sizeof(ciphertext), file);
  fclose(file);

  if (ciphertext_len <= 0) {
    fprintf(stderr, "Failed to read session data.\n");
    return -1;
  }

  // Decrypt the ciphertext
  unsigned char decryptedtext[512];
  int decryptedtext_len =
      decrypt_session(ciphertext, ciphertext_len, key, iv, decryptedtext);
  if (decryptedtext_len < 0) {
    fprintf(stderr, "Decryption failed.\n");
    return -1;
  }
  decryptedtext[decryptedtext_len] = '\0';

  // Parse decrypted data
  sscanf((char *)decryptedtext, "%s\n%s\n", username, session_token);
  return 0;
}

// Delete session file securely
int delete_session() {
  if (remove("session.txt") == 0) {
    printf("Session deleted successfully.\n");
    return 0;
  } else {
    fprintf(stderr, "Failed to delete session file.\n");
    return -1;
  }
}
