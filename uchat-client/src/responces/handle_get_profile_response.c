#include <uchat.h>

void update_username_label(AppData *app_data, const char *username) {
    gtk_label_set_text(GTK_LABEL(app_data->main_page->profile_data.username), username);
}

int handle_get_profile_response(cJSON *response, AppData *app_data) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (status && strcmp(status->valuestring, "SUCCESS") != 0) {
        fprintf(stderr, "Error: Server response status is not SUCCESS.\n");
        return 1;
    }

    cJSON *username_item = cJSON_GetObjectItem(response, "username");
    if (username_item && cJSON_IsString(username_item)) { 

        update_username_label(app_data, username_item->valuestring);

        return 0;  
    } else {
        fprintf(stderr, "Invalid or missing username in response.\n");
        return 1;  
    }
}
