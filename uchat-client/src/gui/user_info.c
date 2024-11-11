#include <uchat.h>

// change profile
void change_profile(GtkWidget *change_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username =
      (char *)gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.username));
  if (check_form_data(username, NULL, (*main_page).edit_data.message) == 1) {
    gtk_label_set_label(
        GTK_LABEL((*main_page).profile_data.username),
        gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.username)));
    gtk_label_set_label(
        GTK_LABEL((*main_page).profile_data.description),
        gtk_entry_get_text(GTK_ENTRY((*main_page).edit_data.description)));
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                     "user_info");
  }
}

