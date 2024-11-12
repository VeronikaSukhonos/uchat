#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <uchat.h>

// Ensure the cache directory exists
void ensure_cache_directory() {
  struct stat st = {0};
  if (stat(CACHE_DIR, &st) == -1) {
    mkdir(CACHE_DIR, 0700);
  }
}

// Get the full path for a message file
void get_message_file_path(char *path, size_t size, const char *chat_id) {
  snprintf(path, size, CACHE_DIR "/" MESSAGE_FILE_FORMAT, chat_id);
}
// Get the full path for a voice message file
void get_voice_file_path(char *path, size_t size, const char *chat_id) {
  snprintf(path, size, CACHE_DIR "/" VOICE_FILE_FORMAT, chat_id);
}

// Append a MessageCache to the linked list
MessageNode *append_message_node(MessageNode *head, MessageCache message) {
  MessageNode *new_node = (MessageNode *)malloc(sizeof(MessageNode));
  if (!new_node) {
    perror("Failed to allocate memory for new node");
    return head;
  }
  new_node->message = message;
  new_node->next = NULL;

  if (!head) {
    return new_node;
  }

  MessageNode *current = head;
  while (current->next) {
    current = current->next;
  }
  current->next = new_node;
  return head;
}

// Save an encrypted message to the cache file
int save_encrypted_message_to_cache(const char *chat_id,
                                    const MessageCache *message) {
  ensure_cache_directory();
  unsigned char key[KEY_SIZE];
  unsigned char iv[IV_SIZE];

  char serial[128];
  get_serial_number(
      serial, sizeof(serial)); // Get the unique serial number for the device

  if (derive_key_from_serial(serial, key) != 0) {
    fprintf(stderr, "Failed to derive encryption key.\n");
    return -1;
  }

  char message_file_path[256];
  get_message_file_path(message_file_path, sizeof(message_file_path), chat_id);

  unsigned char plaintext[2048];
  snprintf((char *)plaintext, sizeof(plaintext),
           "MessageID: %s\nSender: %s\nDate: %ld\nStatus: %d\nContentType: "
           "%d\nContent: %s\nVoicePath: %s\n",
           message->message_id, message->sender, message->date, message->status,
           message->content_type, message->content, message->voice_path);

  unsigned char ciphertext[2048];
  if (RAND_bytes(iv, IV_SIZE) != 1) {
    fprintf(stderr, "Failed to generate IV.\n");
    return -1;
  }

  int ciphertext_len = encrypt_session(plaintext, strlen((char *)plaintext),
                                       key, ciphertext, iv);
  if (ciphertext_len < 0) {
    fprintf(stderr, "Encryption failed.\n");
    return -1;
  }

  FILE *file =
      fopen(message_file_path, "ab"); // Append to file for multiple messages
  if (!file) {
    perror("Failed to open message cache file");
    return -1;
  }

  fwrite(iv, 1, IV_SIZE, file);                // Write IV for decryption
  fwrite(ciphertext, 1, ciphertext_len, file); // Write encrypted message
  fclose(file);

  return 0;
}

// Load all encrypted messages from cache into a linked list
MessageNode *load_encrypted_messages_from_cache(const char *chat_id) {
  unsigned char key[KEY_SIZE];
  unsigned char iv[IV_SIZE];
  char message_file_path[256];
  get_message_file_path(message_file_path, sizeof(message_file_path), chat_id);

  char serial[128];
  get_serial_number(serial, sizeof(serial));

  if (derive_key_from_serial(serial, key) != 0) {
    fprintf(stderr, "Failed to derive encryption key.\n");
    return NULL;
  }

  FILE *file = fopen(message_file_path, "rb");
  if (!file) {
    perror("Failed to open message cache file");
    return NULL;
  }

  MessageNode *head = NULL;
  while (fread(iv, 1, IV_SIZE, file) == IV_SIZE) {
    unsigned char ciphertext[2048];
    int ciphertext_len = fread(ciphertext, 1, sizeof(ciphertext), file);
    if (ciphertext_len <= 0) {
      break;
    }

    unsigned char plaintext[2048];
    int plaintext_len =
        decrypt_session(ciphertext, ciphertext_len, key, iv, plaintext);
    if (plaintext_len < 0) {
      fprintf(stderr, "Decryption failed.\n");
      break;
    }
    plaintext[plaintext_len] = '\0';

    // Parse decrypted data into a MessageCache struct
    MessageCache message;
    sscanf((char *)plaintext,
           "MessageID: %63s\nSender: %63s\nDate: %ld\nStatus: %d\nContentType: "
           "%d\nContent: %1023[^\n]\nVoicePath: %255[^\n]",
           message.message_id, message.sender, &message.date,
           (int *)&message.status, (int *)&message.content_type,
           message.content, message.voice_path);

    head = append_message_node(head, message);
  }

  fclose(file);
  return head;
}

// Free the linked list
void free_message_list(MessageNode *head) {
  while (head) {
    MessageNode *temp = head;
    head = head->next;
    free(temp);
  }
}

// Print all messages in the linked list (for testing)
void print_messages(MessageNode *head) {
  MessageNode *current = head;
  while (current) {
    MessageCache *msg = &current->message;
    printf("MessageID: %s, Sender: %s, Date: %s, Status: %d, ContentType: %d, "
           "Content: %s\n",
           msg->message_id, msg->sender, ctime(&msg->date), msg->status,
           msg->content_type, msg->content);
    current = current->next;
  }
}

// // Example usage
// int main() {
//   const char *chat_id = "12345";

//   MessageCache message = {.message_id = "msg_001",
//                           .sender = "user_123",
//                           .date = time(NULL),
//                           .content_type = TEXT,
//                           .status = NEW,
//                           .content = "Hello, this is a text message.",
//                           .voice_path = ""};

//   if (save_encrypted_message_to_cache(chat_id, &message) == 0) {
//     printf("Message cached securely.\n");
//   }

//   MessageNode *messages = load_encrypted_messages_from_cache(chat_id);
//   if (messages) {
//     printf("Loaded messages:\n");
//     print_messages(messages);
//     free_message_list(messages);
//   } else {
//     printf("No messages found for chat ID %s.\n", chat_id);
//   }

//   return 0;
// }