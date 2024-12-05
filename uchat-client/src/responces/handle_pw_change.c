#include <uchat.h>

void handle_update_password_response(cJSON *response, t_main_page_data *main_page) {
    cJSON *status = cJSON_GetObjectItem(response, "status");

    if (!status || !cJSON_IsString(status)) {
        success_or_error_msg(main_page->change_pw.message,
                             "Invalid server response.", FALSE);
        return;
    }

    if (strcmp(status->valuestring, "SUCCESS") == 0) {
        success_or_error_msg(main_page->change_pw.message,
                             "Password changed successfully!", TRUE);
        

        // reset fields
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.old_pw), "");
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw), "");
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw_again), "");
        switch_to_page_with_delay(main_page, "settings", 1000);
    } else {
        cJSON *error_msg = cJSON_GetObjectItem(response, "error");
        const char *message = error_msg && cJSON_IsString(error_msg) ? error_msg->valuestring : "Password change failed.";
        success_or_error_msg(main_page->change_pw.message, message, FALSE);

        // reset fields 
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.old_pw), "");
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw), "");
        gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw_again), "");
    }
}