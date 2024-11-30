#include <uchat_server.h>

cJSON *retrieve_chat_details(sqlite3 *db, int chat_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT id, name, type FROM chats WHERE id = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for chat details: %s\n",
            sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, chat_id);

  cJSON *chat = NULL;

  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const char *chat_name = (const char *)sqlite3_column_text(stmt, 1);
    const char *chat_type = (const char *)sqlite3_column_text(stmt, 2);

    chat = cJSON_CreateObject();
    cJSON_AddNumberToObject(chat, "chat_id", chat_id);
    cJSON_AddStringToObject(chat, "name", chat_name ? chat_name : "Unknown");
    cJSON_AddStringToObject(chat, "type", chat_type ? chat_type : "unknown");

    // Retrieve last 20 messages for this chat
    cJSON *messages = retrieve_last_20_messages(db, chat_id);
    if (messages) {
      cJSON_AddItemToObject(chat, "messages", messages);
    }

    // Retrieve members for this chat
    cJSON *members = retrieve_chat_members(db, chat_id);
    if (members) {
      cJSON_AddItemToObject(chat, "members", members);
    }
  }

  sqlite3_finalize(stmt);

  return chat;
}

void handle_get_new_data_request(sqlite3 *db, Client *client,
                                 cJSON *client_request) {
  // Step 1: Get cached chat IDs from the client request
  cJSON *cached_chats = cJSON_GetObjectItem(client_request, "cached_chats");

  int client_sock = client->socket;
  int user_id = get_user_id(db, client->username);

  // Query the database for all chat IDs the user is part of
  sqlite3_stmt *stmt;
  const char *sql = "SELECT chat_id as id FROM chat_members WHERE user_id = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for fetching chat IDs: %s\n",
            sqlite3_errmsg(db));
    return;
  }

  sqlite3_bind_int(stmt, 1, user_id); // Bind the user ID

  cJSON *missing_chats = cJSON_CreateArray();
  cJSON *undelivered_messages = cJSON_CreateObject();

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int chat_id = sqlite3_column_int(stmt, 0);
    printf("Found on server chat_id: %d\n", chat_id);
    int found_in_cache = 0;

    // Check if the chat_id is in the client's cached list
    cJSON *chat_item;
    cJSON_ArrayForEach(chat_item, cached_chats) {
      if (chat_id == chat_item->valueint) {
        printf("Found in cache chat_id: %d\n", chat_id);
        found_in_cache = 1;

        // Fetch undelivered messages for this existing chat
        cJSON *messages_for_chat =
            retrieve_undelivered_messages(db, user_id, chat_id);
        if (messages_for_chat) {
          char chat_id_str[20];
          snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
          cJSON_AddItemToObject(undelivered_messages, chat_id_str,
                                messages_for_chat);
        }
        break;
      }
    }

    // If the chat is missing, fetch its details and add it to missing chats
    if (!found_in_cache) {
      cJSON *chat_details = retrieve_chat_details(db, chat_id);
      if (chat_details) {
        cJSON_AddItemToArray(missing_chats, chat_details);

        // Fetch undelivered messages for this missing chat
        cJSON *messages_for_chat =
            retrieve_undelivered_messages(db, user_id, chat_id);
        if (messages_for_chat) {
          char chat_id_str[20];
          snprintf(chat_id_str, sizeof(chat_id_str), "%d", chat_id);
          cJSON_AddItemToObject(undelivered_messages, chat_id_str,
                                messages_for_chat);
        }
      }
    }
  }

  sqlite3_finalize(stmt);

  // Step 3: Send the response with missing chats and undelivered messages
  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "GET_NEW_DATA");
  cJSON_AddItemToObject(response, "missing_chats", missing_chats);
  cJSON_AddItemToObject(response, "undelivered_messages", undelivered_messages);

  char *response_str = cJSON_Print(response);
  send(client_sock, response_str, strlen(response_str), 0);

  printf("Sent: %s\n", response_str);

  // Cleanup
  free(response_str);
  cJSON_Delete(response);
}