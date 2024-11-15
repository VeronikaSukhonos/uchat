#include <database.h>

// Function to retrieve the members of a chat
cJSON *retrieve_chat_members(sqlite3 *db, int chat_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT u.id, u.username "
                    "FROM chat_members cm "
                    "JOIN users u ON cm.user_id = u.id "
                    "WHERE cm.chat_id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for chat members: %s\n",
            sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, chat_id);

  // Create JSON array for members
  cJSON *members = cJSON_CreateArray();
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int member_id = sqlite3_column_int(stmt, 0);
    const char *username = (const char *)sqlite3_column_text(stmt, 1);

    cJSON *member = cJSON_CreateObject();
    cJSON_AddNumberToObject(member, "user_id", member_id);
    cJSON_AddStringToObject(member, "username", username);

    cJSON_AddItemToArray(members, member);
  }
  sqlite3_finalize(stmt);

  return members;
}

// Function to retrieve the chat list along with members and last 20 messages
cJSON *retrieve_chat_list(sqlite3 *db, int user_id) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT c.id, c.name, c.type "
                    "FROM chats c "
                    "JOIN chat_members cm ON c.id = cm.chat_id "
                    "WHERE cm.user_id = ?;";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement for chat list: %s\n",
            sqlite3_errmsg(db));
    return NULL;
  }

  sqlite3_bind_int(stmt, 1, user_id);

  // Create JSON array for chats
  cJSON *chats = cJSON_CreateArray();
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int chat_id = sqlite3_column_int(stmt, 0);
    const char *chat_name = (const char *)sqlite3_column_text(stmt, 1);
    const char *chat_type = (const char *)sqlite3_column_text(stmt, 2);

    cJSON *chat = cJSON_CreateObject();
    cJSON_AddNumberToObject(chat, "chat_id", chat_id);
    cJSON_AddStringToObject(chat, "name", chat_name);
    cJSON_AddStringToObject(chat, "type", chat_type);

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

    cJSON_AddItemToArray(chats, chat);
  }
  sqlite3_finalize(stmt);

  return chats;
}