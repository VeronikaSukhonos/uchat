#include <cJSON.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <uchat.h>

// Ensure the cache directory exists

char *decrypt_json_from_file(const char *file_path) {
  unsigned char key[KEY_SIZE];
  unsigned char iv[IV_SIZE];
  char serial[128];
  get_serial_number(serial, sizeof(serial));

  // Derive encryption key from the device's serial number
  if (derive_key_from_serial(serial, key) != 0) {
    fprintf(stderr, "Failed to derive encryption key.\n");
    return NULL;
  }

  // Open the encrypted file for reading
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    perror("Failed to open encrypted JSON file");
    return NULL;
  }

  // Read the IV from the file
  fread(iv, 1, IV_SIZE, file);

  // Get the size of the encrypted data in the file (excluding IV)
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file) - IV_SIZE;
  fseek(file, IV_SIZE, SEEK_SET); // Start reading after IV

  // Dynamically allocate memory for the ciphertext based on file size
  unsigned char *ciphertext = g_malloc(file_size);
  if (!ciphertext) {
    fprintf(stderr, "Memory allocation failed for ciphertext.\n");
    fclose(file);
    return NULL;
  }

  // Read the encrypted data (ciphertext) into the buffer
  fread(ciphertext, 1, file_size, file);
  fclose(file);

  // Dynamically allocate memory for the decrypted data (plaintext)
  unsigned char *plaintext = g_malloc(
      file_size); // At worst, plaintext can be the same size as ciphertext
  if (!plaintext) {
    fprintf(stderr, "Memory allocation failed for plaintext.\n");
    g_free(ciphertext);
    return NULL;
  }

  // Decrypt the ciphertext into the plaintext buffer
  int plaintext_len =
      decrypt_session(ciphertext, file_size, key, iv, plaintext);
  g_free(ciphertext);

  if (plaintext_len < 0) {
    fprintf(stderr, "Decryption failed for file: %s\n", file_path);
    g_free(plaintext);
    return NULL;
  }

  // Null-terminate the plaintext
  plaintext[plaintext_len] = '\0';

  // Duplicate the decrypted plaintext and return it as a string
  char *result = strdup((char *)plaintext);
  g_free(plaintext); // Free plaintext after duplicating it

  return result;
}

int remove_directory(const char *path) {
  struct dirent *entry;
  DIR *dir = opendir(path);

  if (!dir) {
    perror("Failed to open directory");
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {
    // Skip "." and ".."
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

    struct stat path_stat;
    if (stat(full_path, &path_stat) == 0) {
      if (S_ISDIR(path_stat.st_mode)) {
        // Recursively remove subdirectory
        remove_directory(full_path);
      } else {
        // Remove file
        if (remove(full_path) != 0) {
          perror("Failed to remove file");
        }
      }
    }
  }

  closedir(dir);

  // Remove the directory itself
  if (rmdir(path) != 0) {
    perror("Failed to remove directory");
    return -1;
  }

  return 0;
}

void delete_cache_directory() {
  if (remove_directory(CACHE_DIR) == 0) {
    printf("Cache directory and its contents removed successfully.\n");
  } else {
    printf("Failed to remove cache directory.\n");
  }
}
void ensure_cache_directory() {
  struct stat st = {0};
  if (stat(CACHE_DIR, &st) == -1) {
    mkdir(CACHE_DIR, 0700);
  }
}

void copy_until_newline_or_max_len(const char *input, char *output,
                                   size_t max_len) {
  size_t i = 0;

  // Iterate over each character of the input string
  while (i < max_len - 1 && input[i] != '\0') {
    if (input[i] == '\n') {
      break; // Stop copying at the newline
    }
    output[i] = input[i]; // Copy the character to the output
    i++;
  }

  output[i] = '\0'; // Ensure the string is null-terminated
}

int read_chat_data_from_encrypted_json(const char *file_path, int *chat_id,
                                       char *name, char *chat_type,
                                       char *last_message, char *last_sender,
                                       char *last_time, char *unread) {
  // Decrypt JSON data from the file
  char *decrypted_json = decrypt_json_from_file(file_path);
  if (!decrypted_json) {
    fprintf(stderr, "Failed to decrypt JSON from file: %s\n", file_path);
    return -1;
  }

  // Parse decrypted JSON
  cJSON *json = cJSON_Parse(decrypted_json);
  // g_print("extracted data: %s\n", decrypted_json);
  g_free(decrypted_json);
  if (!json) {
    fprintf(stderr, "Failed to parse JSON file: %s\n", file_path);
    return -1;
  }

  // Extract chat ID, name, and type
  cJSON *id_json = cJSON_GetObjectItem(json, "chat_id");
  cJSON *name_json = cJSON_GetObjectItem(json, "name");
  cJSON *type_json = cJSON_GetObjectItem(json, "type");
  cJSON *messages = cJSON_GetObjectItem(json, "messages");

  if (!cJSON_IsNumber(id_json) || !cJSON_IsString(name_json) ||
      !cJSON_IsString(type_json)) {
    fprintf(stderr, "Invalid JSON format in file: %s\n", file_path);
    cJSON_Delete(json);
    return -1;
  }

  *chat_id = id_json->valueint;
  strcpy(name, name_json->valuestring);
  strcpy(chat_type, type_json->valuestring);

  // Check if there are any messages
  if (cJSON_IsArray(messages) && cJSON_GetArraySize(messages) > 0) {
    // Get the last message details
    cJSON *last_message_json = cJSON_GetArrayItem(messages, 0);
    cJSON *message_content = cJSON_GetObjectItem(last_message_json, "content");
    cJSON *message_sender = cJSON_GetObjectItem(last_message_json, "username");
    cJSON *message_timestamp =
        cJSON_GetObjectItem(last_message_json, "timestamp");
    cJSON *message_unread = cJSON_GetObjectItem(last_message_json, "read");

    if (cJSON_IsString(message_content) && cJSON_IsString(message_sender) &&
        cJSON_IsString(message_timestamp)) {
      copy_until_newline_or_max_len(message_content->valuestring, last_message,
                                    26);
      strncpy(last_sender, message_sender->valuestring, 63);

      if (strcmp(last_message, "") == 0) {
        strcpy(last_message, "voice message");
      }

      // Parse timestamp using sscanf and convert to local time
      struct tm tm_utc = {0};
      if (sscanf(message_timestamp->valuestring, "%d-%d-%d %d:%d:%d",
                 &tm_utc.tm_year, &tm_utc.tm_mon, &tm_utc.tm_mday,
                 &tm_utc.tm_hour, &tm_utc.tm_min, &tm_utc.tm_sec) == 6) {
        tm_utc.tm_year -= 1900; // Adjust year
        tm_utc.tm_mon -= 1;     // Adjust month (1-12 to 0-11)

        // Convert to time_t and then to local time
        time_t time_utc = timegm(&tm_utc); // timegm for UTC to time_t
        struct tm *tm_local = localtime(&time_utc);
        if (tm_local) {
          strftime(last_time, 31, "%Y-%m-%d %H:%M:%S", tm_local);
        } else {
          strncpy(last_time, "Invalid local time", 31);
          last_time[30] = '\0'; // Ensure null-termination
        }
      } else {
        strncpy(last_time, "Invalid timestamp", 31);
        last_time[30] = '\0'; // Ensure null-termination
      }
    }

    // Set unread status
    if (cJSON_IsNumber(message_unread) && message_unread->valueint == 0) {
      strcpy(unread, "Unread");
    } else {
      strcpy(unread, "");
    }
  } else {
    // No messages available
    strcpy(last_message, "No messages yet");
    strcpy(last_sender, " ");
    strcpy(last_time, "");
    strcpy(unread, "");
  }

  // Clean up and return
  cJSON_Delete(json);
  return 0;
}

// Get the full path for a message file
void get_message_file_path(char *path, size_t size, const char *chat_id) {
  snprintf(path, size, CACHE_DIR "/" MESSAGE_FILE_FORMAT, chat_id);
}

// Example function to get the last message's information

void save_encrypted_chat_to_cache(const char *file_path, cJSON *chat_data) {
  unsigned char key[KEY_SIZE];
  unsigned char iv[IV_SIZE];
  char serial[128];
  get_serial_number(serial, sizeof(serial));

  // Derive encryption key from the device's serial number
  if (derive_key_from_serial(serial, key) != 0) {
    fprintf(stderr, "Failed to derive encryption key.\n");
    return;
  }

  // Generate an IV for encryption
  if (RAND_bytes(iv, IV_SIZE) != 1) {
    fprintf(stderr, "Failed to generate IV.\n");
    return;
  }

  // Convert chat_data to JSON string
  char *json_data = cJSON_Print(chat_data);
  if (!json_data) {
    fprintf(stderr, "Failed to convert chat data to JSON.\n");
    return;
  }

  // Encrypt JSON data
  int json_data_len = strlen(json_data);
  unsigned char *ciphertext = (unsigned char *)g_malloc(
      json_data_len + 16); // Allocate enough space for padding

  if (!ciphertext) {
    fprintf(stderr, "Memory allocation failed\n");
    g_free(json_data);
    return;
  }

  // Call encryption function
  int ciphertext_len = encrypt_session((unsigned char *)json_data,
                                       json_data_len, key, ciphertext, iv);
  g_free(json_data); // Free JSON data after encryption

  if (ciphertext_len < 0) {
    fprintf(stderr, "Encryption failed.\n");
    return;
  }

  // Save IV and encrypted data to file
  FILE *file = fopen(file_path, "wb");
  if (!file) {
    perror("Failed to open chat file for writing");
    return;
  }

  fwrite(iv, 1, IV_SIZE, file);
  fwrite(ciphertext, 1, ciphertext_len, file);
  fclose(file);
  g_free(ciphertext);
}

int insert_message_into_chat(const char *file_path, cJSON *new_message) {
  // Step 1: Decrypt the chat file
  char *decrypted_json = decrypt_json_from_file(file_path);
  if (!decrypted_json) {
    fprintf(stderr, "Failed to decrypt chat file: %s\n", file_path);
    return -1;
  }

  // Step 2: Parse the decrypted JSON
  cJSON *chat_data = cJSON_Parse(decrypted_json);
  g_free(decrypted_json);
  if (!chat_data) {
    fprintf(stderr, "Failed to parse decrypted JSON.\n");
    return -1;
  }

  // Debug: Print the initial chat_data
  g_print("Initial chat_data: %s\n", cJSON_Print(chat_data));

  // Step 3: Get the messages array
  cJSON *messages = cJSON_GetObjectItem(chat_data, "messages");
  if (!cJSON_IsArray(messages)) {
    fprintf(stderr, "Messages array not found or invalid.\n");
    cJSON_Delete(chat_data);
    return -1;
  }

  // Step 4: Insert the new message at the beginning of the array
  cJSON *updated_messages = cJSON_CreateArray();
  cJSON_AddItemToArray(updated_messages, cJSON_Duplicate(new_message, 1));

  // Add existing messages to the updated array
  cJSON *message;
  cJSON_ArrayForEach(message, messages) {
    cJSON_AddItemToArray(updated_messages, cJSON_Duplicate(message, 1));
  }

  // Replace only the "messages" array
  cJSON_ReplaceItemInObject(chat_data, "messages", updated_messages);

  // Debug: Print the updated chat_data
  g_print("Updated chat_data: %s\n", cJSON_Print(chat_data));

  // Ensure the "members" array is preserved
  cJSON *members = cJSON_GetObjectItem(chat_data, "members");
  if (!cJSON_IsArray(members) || cJSON_GetArraySize(members) == 0) {
    fprintf(stderr,
            "WARNING: Members array is missing or empty for chat_id: %d\n",
            cJSON_GetObjectItem(chat_data, "chat_id")->valueint);
  } else {
    g_print("Members array preserved with %d members.\n",
            cJSON_GetArraySize(members));
  }

  // Step 5: Encrypt and save the updated JSON back to the file
  save_encrypted_chat_to_cache(file_path, chat_data);

  // Step 6: Clean up
  cJSON_Delete(chat_data);

  printf("Message inserted successfully into chat file: %s\n", file_path);
  return 0;
}

void create_msg_buttons_from_cache(t_main_page_data *main_page,
                                   const char *cache_dir) {
  // Step 1: Iterate over the chat cache files in the given directory
  DIR *dir = opendir(cache_dir);
  if (!dir) {
    perror("Failed to open cache directory");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Skip non-chat files and hidden files
    if (strstr(entry->d_name, ".json") == NULL) {
      continue;
    }

    // Step 2: Build the full path to the chat file
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", cache_dir, entry->d_name);

    // Step 3: Decrypt and parse the chat data from the file
    char *decrypted_json = decrypt_json_from_file(file_path);
    if (!decrypted_json) {
      fprintf(stderr, "Failed to decrypt chat file: %s\n", file_path);
      continue;
    }

    cJSON *chat_json = cJSON_Parse(decrypted_json);
    g_free(decrypted_json);
    if (!chat_json) {
      fprintf(stderr, "Failed to parse JSON for chat: %s\n", file_path);
      continue;
    }

    // Step 4: Extract chat metadata (chat_id, name, type) from JSON
    cJSON *chat_id_json = cJSON_GetObjectItem(chat_json, "chat_id");
    cJSON *name_json = cJSON_GetObjectItem(chat_json, "name");
    cJSON *type_json = cJSON_GetObjectItem(chat_json, "type");
    cJSON *messages_json = cJSON_GetObjectItem(chat_json, "messages");

    if (!chat_id_json || !name_json || !type_json || !messages_json) {
      fprintf(stderr, "Missing required fields in chat data: %s\n", file_path);
      cJSON_Delete(chat_json);
      continue;
    }

    int chat_id = chat_id_json->valueint;
    const char *chat_name = name_json->valuestring;
    const char *chat_type = type_json->valuestring;

    // Step 5: Process messages in this chat and create MessageNode for each
    cJSON *message_json;
    int messages_size = cJSON_GetArraySize(messages_json); // Get array size
    for (int i = messages_size - 1; i >= 0; i--) {
      // Step 6: Access each message from the end
      message_json = cJSON_GetArrayItem(messages_json, i);

      // Determine the message type (text or voice)
      MessageNode *new_node;
      cJSON *content_type_json = cJSON_GetObjectItem(message_json, "type");
      if (cJSON_IsString(content_type_json) &&
          strcmp(content_type_json->valuestring, "voice") == 0) {
        new_node = create_message_node(main_page, VOICE, chat_id, message_json);
      } else {
        new_node = create_message_node(main_page, TEXT, chat_id, message_json);
      }
    }

    // Step 9: Clean up after processing the chat
    cJSON_Delete(chat_json);
  }

  closedir(dir);
}

void free_message_list(MessageNode *head) {
  MessageNode *current_node = head;
  while (current_node != NULL) {
    MessageNode *next_node = current_node->next; // Save the next node
    if (current_node->message != NULL) {
      g_print("deleted node with id: %i\n", current_node->message->message_id);
      g_free(current_node->message); // Free the associated message data
    }
    g_free(current_node);     // Free the node itself
    current_node = next_node; // Move to the next node
  }
  head = NULL;
}
void get_cached_chat_ids(const char *cache_dir, cJSON *cached_chats) {
  DIR *dir = opendir(cache_dir);
  if (!dir) {
    perror("Failed to open cache directory");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strstr(entry->d_name, ".json") != NULL) {
      // Extract chat ID from the file name (assuming "chat_<chat_id>.json")
      int chat_id;
      sscanf(entry->d_name, "chat_%d.json", &chat_id);

      // Add chat ID to the list
      cJSON_AddItemToArray(cached_chats, cJSON_CreateNumber(chat_id));
    }
  }

  closedir(dir);
}

void merge_chat_data_with_server(const char *file_path, cJSON *new_messages,
                                 int chat_id) {
  // Step 1: Decrypt the chat file
  char *decrypted_json = decrypt_json_from_file(file_path);
  if (!decrypted_json) {
    fprintf(stderr, "Failed to decrypt chat file: %s\n", file_path);
    return;
  }

  // Step 2: Parse the decrypted JSON
  cJSON *chat_data = cJSON_Parse(decrypted_json);
  g_free(decrypted_json);
  if (!chat_data) {
    fprintf(stderr, "Failed to parse decrypted JSON.\n");
    return;
  }

  // Step 3: Get the existing messages array
  cJSON *existing_messages = cJSON_GetObjectItem(chat_data, "messages");
  if (!cJSON_IsArray(existing_messages)) {
    fprintf(stderr, "Messages array not found or invalid in chat file: %s\n",
            file_path);
    cJSON_Delete(chat_data);
    return;
  }

  // Step 4: Create a map of existing message IDs for quick lookup
  cJSON *message;
  cJSON *existing_ids = cJSON_CreateObject();
  cJSON_ArrayForEach(message, existing_messages) {
    int existing_message_id =
        cJSON_GetObjectItem(message, "message_id")->valueint;
    char key[32];
    snprintf(key, sizeof(key), "%d", existing_message_id);
    cJSON_AddNumberToObject(existing_ids, key, 1);
  }

  // Step 5: Create a new array for merged messages
  cJSON *merged_messages = cJSON_CreateArray();

  // Add new messages that are not already in the cache, to the beginning
  cJSON *new_message;
  cJSON_ArrayForEach(new_message, new_messages) {
    int new_message_id =
        cJSON_GetObjectItem(new_message, "message_id")->valueint;
    char key[32];
    snprintf(key, sizeof(key), "%d", new_message_id);

    if (!cJSON_HasObjectItem(existing_ids, key)) {
      cJSON_AddItemToArray(merged_messages, cJSON_Duplicate(new_message, 1));
    }
  }

  // Add existing messages next
  cJSON_ArrayForEach(message, existing_messages) {
    cJSON_AddItemToArray(merged_messages, cJSON_Duplicate(message, 1));
  }

  // Step 6: Replace the messages array in the chat data
  cJSON_ReplaceItemInObject(chat_data, "messages", merged_messages);

  // Step 7: Encrypt and save the updated JSON back to the file
  save_encrypted_chat_to_cache(file_path, chat_data);

  // Cleanup
  cJSON_Delete(existing_ids);
  cJSON_Delete(chat_data);

  printf("Chat data merged and updated successfully for chat_id: %d\n",
         chat_id);
}