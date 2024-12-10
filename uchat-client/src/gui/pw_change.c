#include <uchat.h>

void show_pw(GtkWidget *edit_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  GtkWidget *main_window =
      gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(
          gtk_widget_get_parent(main_page->central_area_stack))));
  gtk_window_set_focus(GTK_WINDOW(main_window), NULL);
  gtk_entry_set_text(GTK_ENTRY((*main_page).change_pw.old_pw), "");
  gtk_entry_set_text(GTK_ENTRY((*main_page).change_pw.new_pw), "");
  gtk_entry_set_text(GTK_ENTRY((*main_page).change_pw.new_pw_again), "");
  gtk_entry_set_visibility(GTK_ENTRY((*main_page).change_pw.old_pw), TRUE);
  gtk_entry_set_visibility(GTK_ENTRY((*main_page).change_pw.new_pw), TRUE);
  gtk_entry_set_visibility(GTK_ENTRY((*main_page).change_pw.new_pw_again),
                           TRUE);
  change_password_visibility((*main_page).change_pw.old_pw_button,
                             (*main_page).change_pw.old_pw);
  change_password_visibility((*main_page).change_pw.new_pw_button,
                             (*main_page).change_pw.new_pw);
  change_password_visibility((*main_page).change_pw.new_pw_again_button,
                             (*main_page).change_pw.new_pw_again);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "edit_password");
}
void send_pw_change_req(GtkWidget *button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  const gchar *old_pw =
      gtk_entry_get_text(GTK_ENTRY(main_page->change_pw.old_pw));
  const gchar *new_pw =
      gtk_entry_get_text(GTK_ENTRY(main_page->change_pw.new_pw));
  const gchar *new_pw_again =
      gtk_entry_get_text(GTK_ENTRY(main_page->change_pw.new_pw_again));
  // Check if any fields are empty
  if (g_strcmp0(old_pw, "") == 0 || g_strcmp0(new_pw, "") == 0 ||
      g_strcmp0(new_pw_again, "") == 0) {
    success_or_error_msg(main_page->change_pw.message,
                         "Please fill in all fields before submitting", FALSE);
    return;
  }

  // Check if new passwords match
  if (g_strcmp0(new_pw, new_pw_again) != 0) {
    success_or_error_msg(main_page->change_pw.message,
                         "New passwords do not match", FALSE);
    return;
  }
  char *new_pw_char = (char *)new_pw;
  if (!check_password(new_pw_char, main_page->change_pw.message)) {
    return;
  }

  cJSON *change_pw_data = cJSON_CreateObject();
  cJSON_AddStringToObject(change_pw_data, "action", "UPDATE_PASSWORD");
  cJSON_AddStringToObject(change_pw_data, "username", username);
  cJSON_AddStringToObject(change_pw_data, "old_pw", old_pw);
  cJSON_AddStringToObject(change_pw_data, "new_pw", new_pw);

  char *json_str = cJSON_Print(change_pw_data);
  cJSON_Delete(change_pw_data);

  // Send data to server
  if (send(main_page->sock, json_str, strlen(json_str), 0) != 0) {
    success_or_error_msg(main_page->change_pw.message,
                         "Failed to change password", FALSE);
    g_free(json_str);
    return;
  }

  // g_print("Sent: %s\n", json_str);
  g_free(json_str);

  // reset fields
  gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.old_pw), "");
  gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw), "");
  gtk_entry_set_text(GTK_ENTRY(main_page->change_pw.new_pw_again), "");
  switch_to_page_with_delay(main_page, "clear_area", 1000);
}
