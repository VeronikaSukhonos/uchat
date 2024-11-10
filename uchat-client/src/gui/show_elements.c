#include <uchat.h>

void show_new_chat(GtkWidget *new_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &new_chat_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_chat");
}

void show_new_group(GtkWidget *new_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &new_group_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "create_group");
}
void show_profile(GtkWidget *settings_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &settings_button);
  // username and description from database
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.username), strcmp((char *)gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)), "") == 0 ? "yrezchyk": gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.status), "online");
  gtk_label_set_label(GTK_LABEL((*main_page).profile_data.description), strcmp((char *)gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)), "") == 0 ? "Yevheniia Rezchyk\n@yrezchyk | KN-423k | Student" : gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)));
  // set visible
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "user_info");
}
// show edit page
void show_edit_page(GtkWidget *edit_button, gpointer data) {
    t_main_page_data *main_page = (t_main_page_data *)data;
    gtk_entry_set_text(GTK_ENTRY((*main_page).edit_data.username), gtk_label_get_label(GTK_LABEL((*main_page).profile_data.username)));
    gtk_entry_set_text(GTK_ENTRY((*main_page).edit_data.description), gtk_label_get_label(GTK_LABEL((*main_page).profile_data.description)));
    gtk_label_set_label(GTK_LABEL((*main_page).edit_data.message), "");
    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack), "edit_profile");
}
void show_chat(GtkWidget *chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &chat_button);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "chat");
}

void show_login(GtkWidget *login_link_button, t_form_data *data) {
    GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));

    gtk_stack_set_visible_child_name(GTK_STACK(pages), "login");
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_parent(pages)), NULL);
    gtk_entry_set_text(GTK_ENTRY(data->username), "");
    gtk_entry_set_text(GTK_ENTRY(data->password), "");
    gtk_entry_set_text(GTK_ENTRY(data->repassword), "");
    gtk_label_set_text(GTK_LABEL(data->message), "");
    gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
    gtk_entry_set_visibility(GTK_ENTRY(data->repassword), TRUE);
    change_password_visibility(data->pw_button, data->password);
    change_password_visibility(data->repw_button, data->repassword);
}

void show_registration(GtkWidget *registration_link_button, t_form_data *data) {
    GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));

    gtk_stack_set_visible_child_name(GTK_STACK(pages), "registration");
    gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_parent(pages)), NULL);
    gtk_entry_set_text(GTK_ENTRY(data->username), "");
    gtk_entry_set_text(GTK_ENTRY(data->password), "");
    gtk_label_set_text(GTK_LABEL(data->message), "");
    gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
    change_password_visibility(data->pw_button, data->password);
}


