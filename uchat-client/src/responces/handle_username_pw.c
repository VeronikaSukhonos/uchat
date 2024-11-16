// #include <uchat.h>

// int check_username(const char *username, GtkWidget *message, int sock) {
//     int username_len = strlen(username);

//     if (username_len == 0) {
//         gtk_label_set_text(GTK_LABEL(message), "Username is required");
//         return 0;
//     } else if (username_len < 2 || username_len > 20) {
//         gtk_label_set_text(GTK_LABEL(message), "Username must contain 2-20 symbols");
//         return 0;
//     }

//     for (int i = 0; i < username_len; i++) {
//         if (isdigit(username[i])) {
//             gtk_label_set_text(GTK_LABEL(message), "Username cannot contain digits");
//             return 0;
//         }
//         if (!isalnum(username[i])) {
//             gtk_label_set_text(GTK_LABEL(message), "Username cannot contain special characters");
//             return 0;
//         }
//         if (!islower(username[i])) {
//             gtk_label_set_text(GTK_LABEL(message), "Username must be all lowercase");
//             return 0;
//         }
//     }

//     // Send FIND_USER request to server
//     cJSON *find_user_request = cJSON_CreateObject();
//     cJSON_AddStringToObject(find_user_request, "action", "FIND_USER");
//     cJSON_AddStringToObject(find_user_request, "username", username);
//     char *find_user_request_str = cJSON_Print(find_user_request);
//     cJSON_Delete(find_user_request);

//     if (send(sock, find_user_request_str, strlen(find_user_request_str), 0) == -1) {
//         gtk_label_set_text(GTK_LABEL(message), "Error connecting to server");
//         free(find_user_request_str);
//         return 0;
//     }
//     free(find_user_request_str);

//     // Receive server response
//     char *response = receive_large_json(sock);
//     if (!response) {
//         gtk_label_set_text(GTK_LABEL(message), "Server error: no response");
//         return 0;
//     }

//     cJSON *response_json = cJSON_Parse(response);
//     free(response);

//     if (!response_json) {
//         gtk_label_set_text(GTK_LABEL(message), "Server error: invalid response");
//         return 0;
//     }

//     cJSON *status = cJSON_GetObjectItem(response_json, "status");
//     if (status && strcmp(status->valuestring, "SUCCESS") == 0) {
//         // Username is valid and available
//         cJSON_Delete(response_json);
//         return 1;
//     } else {
//         gtk_label_set_text(GTK_LABEL(message), "Username already exists");
//         cJSON_Delete(response_json);
//         return 0;
//     }
// }
