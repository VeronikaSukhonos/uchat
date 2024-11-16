#include <uchat.h>

void update_username_label(AppData *app_data, const char *username) {
  gtk_label_set_text(GTK_LABEL(app_data->main_page->profile_data.username),
                     username);
}
int handle_get_profile_response(cJSON *response, AppData *app_data) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (status && strcmp(status->valuestring, "SUCCESS") != 0) {
        fprintf(stderr, "Error: Server response status is not SUCCESS.\n");
        return 1;
    }

    cJSON *username = cJSON_GetObjectItem(response, "username");
    cJSON *full_name = cJSON_GetObjectItem(response, "full_name");
    cJSON *group = cJSON_GetObjectItem(response, "group");
    cJSON *role = cJSON_GetObjectItem(response, "role");

    if (!username || !full_name || !role) {
        fprintf(stderr, "Error: Missing required profile data in the response.\n");
        return 1;
    }

    update_username_label(app_data, username->valuestring);

    // Build the description string dynamically
    char description[256] = {0};
    if (group && strlen(group->valuestring) > 0) {
        snprintf(description, sizeof(description), "%s\n@%s | %s | %s",
                 full_name->valuestring, username->valuestring,
                 group->valuestring, role->valuestring);
    } else {
        snprintf(description, sizeof(description), "%s\n@%s | %s",
                 full_name->valuestring, username->valuestring,
                 role->valuestring);
    }

    gtk_label_set_label(GTK_LABEL(app_data->main_page->profile_data.description),
                        description);

    gtk_label_set_label(GTK_LABEL(app_data->main_page->profile_data.username),
                        username->valuestring);
    gtk_label_set_label(
        GTK_LABEL(app_data->main_page->profile_data.student_group),
        group && strlen(group->valuestring) > 0 ? group->valuestring : "");
    gtk_label_set_label(GTK_LABEL(app_data->main_page->profile_data.name_surname),
                        full_name->valuestring);
    gtk_stack_set_visible_child_name(
        GTK_STACK(app_data->main_page->central_area_stack), "user_info");

    return 0;
}
