#include <uchat_server.h>

void handle_request(Client *client, char *buffer, Client clients[],
                    int max_clients, sqlite3 *db) {
  printf("Received: %s\n", buffer);

  // Parse JSON using cJSON
  cJSON *json = cJSON_Parse(buffer);
  if (json == NULL) {
    printf("Invalid JSON received\n");
    return;
  }

  cJSON *action = cJSON_GetObjectItem(json, "action");

  // Check for action and handle accordingly
  if (strcmp(action->valuestring, "LOGIN") == 0) {
    // Handle login

    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    char session_token[33];
    if (handle_login(db, json, session_token, client) == 0) {
      cJSON *username = cJSON_GetObjectItem(json, "username");
      if (username && cJSON_IsString(username)) {
        strcpy(client->username, username->valuestring);
        printf("Stored username: %s\n",
               client->username); // bind username to current client
        send_json_responce_to_client(
            client, build_json_login(username->valuestring, session_token));
      }
    } else {
      send_status_responce_to_client(client, "LOGIN", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "REGISTER") == 0) {
    // Handle register
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_register(db, json) == 0) {
      send_status_responce_to_client(client, "REGISTER", "SUCCESS");
    } else {
      send_status_responce_to_client(client, "REGISTER", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "LOGOUT") == 0) {
    // Handle logout
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_logout(db, client->username) == 0) {
      send_status_responce_to_client(client, "LOGOUT", "SUCCESS");
    } else {
      send_status_responce_to_client(client, "LOGOUT", "FAILURE");
    }
    sqlite3_close(db);

  } else if (strcmp(action->valuestring, "FIND_USER") == 0) {
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_find_user(db, json, client) == 0) {
    } else {
      send_status_responce_to_client(client, "FIND_USER", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "UPDATE_PROFILE_DATA") == 0) {
    // Handle profile update
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    handle_update_profile(db, client, json);
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "DELETE_ACCOUNT") == 0) {
    // Handle account deletion
  } else if (strcmp(action->valuestring, "SEND_MESSAGE_TO_CHAT") == 0) {
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_send_message_to_chat(db, client, json, clients, max_clients) ==
        0) {
      printf("Message sent to chat ID %d\n",
             cJSON_GetObjectItem(json, "chat_id")->valueint);
    } else {
      send_status_responce_to_client(client, "SEND_MESSAGE_TO_CHAT", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "EDIT_MESSAGE") == 0) {
    // Handle editing a previously sent message
  } else if (strcmp(action->valuestring, "DELETE_MESSAGE") == 0) {
    // Handle deleting a previously sent message
  } else if (strcmp(action->valuestring, "READ_RECEIPT") == 0) {
    // Handle sending a read receipt
  } else if (strcmp(action->valuestring, "CREATE_CHAT") == 0) {
    // Handle creating a new chat
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_create_chat(db, json, client, clients, max_clients) == 0) {
    } else {
      send_status_responce_to_client(client, "CREATE_CHAT", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "CREATE_GROUP_CHAT") == 0) {
    // Handle creating a new group chat
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    if (handle_create_group_chat(db, json, client, clients, max_clients) == 0) {
    } else {
      send_status_responce_to_client(client, "CREATE_CHAT", "FAILURE");
    }
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "ADD_MEMBER") == 0) {
    // Handle adding a member to a group
  } else if (strcmp(action->valuestring, "REMOVE_MEMBER") == 0) {
    // Handle removing a member from a group
  } else if (strcmp(action->valuestring, "LEAVE_GROUP") == 0) {
    // Handle leaving a group
  } else if (strcmp(action->valuestring, "GET_CHAT_LIST") == 0) {
    // Handle retrieving a list of chats
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    handle_get_chat_list(db, client);
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "PULL_NEW_NOTIFICATIONS") == 0) {
    // Handle pulling new notifications
  } else if (strcmp(action->valuestring, "SEND_FILE") == 0) {
    // Handle sending a file
  } else if (strcmp(action->valuestring, "RECEIVE_FILE") == 0) {
    // Handle receiving a file
  } else if (strcmp(action->valuestring, "UPLOAD_MEDIA") == 0) {
    // Handle uploading media files
  } else if (strcmp(action->valuestring, "DOWNLOAD_MEDIA") == 0) {
    // Handle downloading media files
  } else if (strcmp(action->valuestring, "CHECK_SESSION") == 0) {
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    char session_token[33];
    if (handle_check_session(db, json, session_token, client) == 0) {
      cJSON *username = cJSON_GetObjectItem(json, "username");
      if (username && cJSON_IsString(username)) {
        strcpy(client->username, username->valuestring);
        printf("Stored username: %s\n",
               client->username); // bind username to current client
        send_json_responce_to_client(
            client, build_json_login(username->valuestring, session_token));
      }
    } else {
      send_status_responce_to_client(client, "LOGIN", "FAILURE");
    }
    sqlite3_close(db);
    // Handle checking session status
  } else if (strcmp(action->valuestring, "GET_PROFILE_DATA") == 0) {
    if (open_database(&db) != 0) {
      fprintf(stderr, "Failed to open database.\n");
    }
    handle_get_profile(db, client);
    sqlite3_close(db);
  } else if (strcmp(action->valuestring, "UPDATE_PASSWORD") == 0) {
    // Handle updating the user's password
  } else if (strcmp(action->valuestring, "RESET_PASSWORD") == 0) {
    // Handle password reset
  } else if (strcmp(action->valuestring, "PIN_MESSAGE") == 0) {
    // Handle pinning a message
  } else if (strcmp(action->valuestring, "MUTE_CHAT") == 0) {
    // Handle muting a chat
  } else if (strcmp(action->valuestring, "UNMUTE_CHAT") == 0) {
    // Handle unmuting a chat
  } else if (strcmp(action->valuestring, "BLOCK_USER") == 0) {
    // Handle blocking a user
  } else if (strcmp(action->valuestring, "UNBLOCK_USER") == 0) {
    // Handle unblocking a user
  } else if (strcmp(action->valuestring, "REACT_TO_MESSAGE") == 0) {
    // Handle reacting to a message
  } else {
    printf("Unknown action received: %s\n", action->valuestring);
  }

  cJSON_Delete(json); // Free the JSON object
}
