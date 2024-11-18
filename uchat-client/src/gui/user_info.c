#include <uchat.h>

void change_profile(GtkWidget *change_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Retrieve text from input fields
  char *username =
      (char *)gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.username));
  char *name_surname = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).edit_data.name_surname));
  char *student_group = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).edit_data.student_group));
  const char *role = gtk_combo_box_text_get_active_text(
      GTK_COMBO_BOX_TEXT((*main_page).edit_data.role_combo));

  // Check if username is valid (adjust validation as needed)
  if (check_form_data(username, NULL, (*main_page).edit_data.message) == 1) {
    // gtk_label_set_label(GTK_LABEL((*main_page).profile_data.username),
    //                     username);
    // gtk_label_set_label(GTK_LABEL((*main_page).profile_data.description),
    //                     g_strdup_printf("%s\n@%s | %s | %s", name_surname,
    //                     username, student_group, role));

    // Switch to the user info page
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "UPDATE_PROFILE_DATA");
    cJSON_AddStringToObject(json, "username", username ? username : "");
    cJSON_AddStringToObject(json, "full_name",
                            name_surname ? name_surname : "");
    cJSON_AddStringToObject(json, "group", student_group ? student_group : "");
    cJSON_AddStringToObject(json, "role", role ? role : "");
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);
    send(main_page->sock, json_str, strlen(json_str), 0);
    g_print("Sent: %s\n", json_str);
    free(json_str);
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                     "user_info");
  }
}
void change_password(GtkWidget *change_button, gpointer data) {
    t_main_page_data *main_page = (t_main_page_data *)data;

    // Retrieve text from input fields
    char *new_pw = (char *)gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.new_pw));
    char *new_pw_again = (char *)gtk_entry_get_text(
        GTK_ENTRY((*main_page).edit_data.new_pw_again));
    // Check if the passwords match
    if (strcmp(new_pw, new_pw_again) != 0) {
        gtk_label_set_text(GTK_LABEL((*main_page).edit_data.message), "Passwords do not match.");
        return; // Exit the function if passwords do not match
    }

        if (check_password(new_pw, (*main_page).edit_data.message)) {
            // Switch to the user info page
            cJSON *json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "action", "CHANGE_PASSWORD_DATA");
            cJSON_AddStringToObject(json, "new_pass", new_pw ? new_pw : "");
            char *json_str = cJSON_Print(json);
            cJSON_Delete(json);
            send(main_page->sock, json_str, strlen(json_str), 0);
            g_print("Sent: %s\n", json_str);
            free(json_str);
            gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                             "user_info");
            gtk_label_set_text(GTK_LABEL((*main_page).edit_data.message), "Password changed successfully!");
        }
}
