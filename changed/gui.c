// sudo apt install libgtk-3-dev
// clang gui.c -o gui `pkg-config --cflags --libs gtk+-3.0`

#include <uchat.h>

void load_css(const gchar *file) {
  GtkCssProvider *provider = gtk_css_provider_new();

  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_css_provider_load_from_path(provider, file, NULL);

  g_object_unref(provider);
}

void show_login(GtkWidget *login_link_button, gpointer data) {
  t_form_data *user_data = (t_form_data *)data;
  GtkWidget *pages =
      gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));

  gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
  gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
  gtk_entry_set_text(GTK_ENTRY(user_data->repassword), "");
  gtk_label_set_text(GTK_LABEL(user_data->message), "");
  gtk_stack_set_visible_child_name(GTK_STACK(pages), "login");
}

void show_registration(GtkWidget *registration_link_button, gpointer data) {
  t_form_data *user_data = (t_form_data *)data;
  GtkWidget *pages =
      gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));

  gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
  gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
  gtk_label_set_text(GTK_LABEL(user_data->message), "");
  gtk_stack_set_visible_child_name(GTK_STACK(pages), "registration");
}

void registration_submit(GtkWidget *registration_button, gpointer data) {
  t_form_data *user_data = (t_form_data *)data;
  GtkWidget *pages =
      gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));
  char *username = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->username));
  char *password = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->password));
  char *repassword =
      (char *)gtk_entry_get_text(GTK_ENTRY(user_data->repassword));

  if (check_form_data(username, password, user_data->message) == 1) {
    if (strcmp(password, repassword) != 0)
      gtk_label_set_text(GTK_LABEL(user_data->message),
                         "Passwords must be the same");
    else {
      // send JSON REGISTER
      char *json_str = build_json_register(username, password);
      g_print("Sock: %i", user_data->sock);
      send(user_data->sock, json_str, strlen(json_str), 0);
      free(json_str);
      g_print("Registration sended to server:\nUsername: %s\nPassword: "
              "%s\nRepassword: %s\n",
              username, password, repassword);
      gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
      gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
      gtk_entry_set_text(GTK_ENTRY(user_data->repassword), "");
      gtk_label_set_text(GTK_LABEL(user_data->message), "");
      // gtk_stack_set_visible_child_name(GTK_STACK(pages), "chats");
    }
  }
}

void login_submit(GtkWidget *login_button, gpointer data) {
  t_form_data *user_data = (t_form_data *)data;
  GtkWidget *pages =
      gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));
  char *username = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->username));
  char *password = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->password));

  if (check_form_data(username, password, user_data->message) == 1) {
    // send JSON LOGIN
    char serial_number[64] = {0};
    get_serial_number(serial_number, sizeof(serial_number));
    char *json_str = build_json_login(username, password, serial_number);
    g_print("Sock: %i", user_data->sock);
    send(user_data->sock, json_str, strlen(json_str), 0);
    free(json_str);
    g_print("Login sended to server:\nUsername: %s\nPassword: %s\n", username,
            password);
    gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
    gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
    gtk_label_set_text(GTK_LABEL(user_data->message), "");
    // gtk_stack_set_visible_child_name(GTK_STACK(pages), "chats");
  }
}

int check_form_data(char *username, char *password, GtkWidget *message) {
  int username_len = strlen(username);
  int password_len = password != NULL ? strlen(password) : -1; /* this function is used to check
                                                               user's nick when creating a chat/group
                                                               and in this case password == NULL*/

  if (username_len == 0) {
    gtk_label_set_text(GTK_LABEL(message), "Username is required");
    return 0;
  } else if (username_len < 2 || username_len > 20) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Username must contain 2-20 symbols");
    return 0;
  } else if (password != NULL && password_len == 0) {
    gtk_label_set_text(GTK_LABEL(message), "Password is required");
    return 0;
  } else if (password != NULL && (password_len < 0 || password_len > 20)) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Password must contain 8-20 symbols");
    return 0;
  }
  return 1;
}
