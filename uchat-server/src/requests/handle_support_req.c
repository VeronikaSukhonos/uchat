#include <uchat_server.h>
int handle_support_request(Client *client, cJSON *json) {
    cJSON *username = cJSON_GetObjectItem(json, "username");
    cJSON *email = cJSON_GetObjectItem(json, "email");

    if (!username || !cJSON_IsString(username)) {
        fprintf(stderr, "Invalid or missing username in SUPPORT_REQUEST.\n");
        send_status_responce_to_client(client, "SUPPORT_REQUEST", "FAILURE");
        return 1;
    }

    if (!email || !cJSON_IsString(email)) {
        fprintf(stderr, "Invalid or missing email in SUPPORT_REQUEST.\n");
        send_status_responce_to_client(client, "SUPPORT_REQUEST", "FAILURE");
        return 1; 
    }

    printf("Received support request from user: %s, email: %s\n",
           username->valuestring, email->valuestring);

    send_status_responce_to_client(client, "SUPPORT_REQUEST", "SUCCESS");
    return 0;
}
