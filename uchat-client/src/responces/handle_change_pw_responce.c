#include <uchat.h>
void handle_up_pw_response(cJSON *response, AppData *app_data) {
    cJSON *status = cJSON_GetObjectItem(response, "status");
    if (status && strcmp(status->valuestring, "SUCCESS") == 0) {
        g_print("Password updated successfully.\n");
        gtk_style_context_add_class(
            gtk_widget_get_style_context(app_data->main_page->change_pw.message),
            "form-message-success");
        gtk_label_set_text(GTK_LABEL(app_data->main_page->change_pw.message),
                           "Password changed successfully!");
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.old_pw), "");
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.new_pw), "");
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.new_pw_again), "");
    } else {
        g_print("Error: Failed to update password.\n");
        gtk_style_context_remove_class(
            gtk_widget_get_style_context(app_data->main_page->change_pw.message),
            "form-message-success");
        gtk_label_set_text(GTK_LABEL(app_data->main_page->change_pw.message),
                           "Failed to update password. Please try again.");
        // Reset password fields
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.old_pw), "");
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.new_pw), "");
        gtk_entry_set_text(GTK_ENTRY(app_data->main_page->change_pw.new_pw_again), "");
    }
}
