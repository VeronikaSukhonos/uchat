#include <uchat_server.h>

void handle_request(Client *client, char *buffer, Client clients[], int max_clients) {
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
        cJSON *username = cJSON_GetObjectItem(json, "username");
        if (username && cJSON_IsString(username)) {
            strcpy(client->username, username->valuestring);
            printf("Stored username: %s\n", client->username);  // bind username to current client
            send_message_to_client(clients, "Login successful", client->username,
                                   max_clients);  // need to send JSON, so remake
        }
    } else if (strcmp(action->valuestring, "REGISTER") == 0) {
        // Handle register
    } else if (strcmp(action->valuestring, "LOGOUT") == 0) {
        // Handle logout
    } else if (strcmp(action->valuestring, "UPDATE_PROFILE") == 0) {
        // Handle profile update
    } else if (strcmp(action->valuestring, "DELETE_ACCOUNT") == 0) {
        // Handle account deletion
    } else if (strcmp(action->valuestring, "SEND_MESSAGE_TO_USER") == 0) {
        // Handle sending a message to another user
        cJSON *receiver = cJSON_GetObjectItem(json, "receiver");
        cJSON *message = cJSON_GetObjectItem(json, "message");
        char send[100];
        strcpy(send, client->username);
        strcat(send, message->valuestring);

        send_message_to_client(clients, send, receiver->valuestring,
                               max_clients);  // need to send JSON, so remake
    } else if (strcmp(action->valuestring, "EDIT_MESSAGE") == 0) {
        // Handle editing a previously sent message
    } else if (strcmp(action->valuestring, "DELETE_MESSAGE") == 0) {
        // Handle deleting a previously sent message
    } else if (strcmp(action->valuestring, "READ_RECEIPT") == 0) {
        // Handle sending a read receipt
    } else if (strcmp(action->valuestring, "CREATE_CHAT") == 0) {
        // Handle creating a new chat
    } else if (strcmp(action->valuestring, "CREATE_GROUP") == 0) {
        // Handle creating a new group chat
    } else if (strcmp(action->valuestring, "ADD_MEMBER") == 0) {
        // Handle adding a member to a group
    } else if (strcmp(action->valuestring, "REMOVE_MEMBER") == 0) {
        // Handle removing a member from a group
    } else if (strcmp(action->valuestring, "LEAVE_GROUP") == 0) {
        // Handle leaving a group
    } else if (strcmp(action->valuestring, "GET_CHAT_LIST") == 0) {
        // Handle retrieving a list of chats
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
        // Handle checking session status
    } else if (strcmp(action->valuestring, "REFRESH_SESSION") == 0) {
        // Handle refreshing a session token
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

    cJSON_Delete(json);  // Free the JSON object
}
