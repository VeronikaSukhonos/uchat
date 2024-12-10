#include <uchat.h>

void load_css(const gchar *file) {
  GtkCssProvider *provider = gtk_css_provider_new();
  gboolean loaded = gtk_css_provider_load_from_path(provider, file, NULL);

  if (!loaded) {
    g_warning("Failed to load CSS file: %s", file);
  }
  gtk_style_context_add_provider_for_screen(
      gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_css_provider_load_from_path(provider, file, NULL);

  g_object_unref(provider);
}

gboolean change_entry_box_focus(GtkWidget *entry, GdkEventFocus *event,
                                GtkWidget *entry_box) {
  if (event->in) {
    gtk_style_context_add_class(gtk_widget_get_style_context(entry_box),
                                "focus");
  } else {
    gtk_style_context_remove_class(gtk_widget_get_style_context(entry_box),
                                   "focus");
  }
  return FALSE;
}

void change_password_visibility(GtkWidget *pw_button, GtkWidget *pw_entry) {
  if (gtk_entry_get_visibility(GTK_ENTRY(pw_entry))) {
    GtkWidget *show_pw_image =
        gtk_image_new_from_file("uchat-client/src/gui/resources/eye-open.png");

    gtk_button_set_image(GTK_BUTTON(pw_button), show_pw_image);
    gtk_entry_set_visibility(GTK_ENTRY(pw_entry), FALSE);
  } else {
    GtkWidget *hide_pw_image =
        gtk_image_new_from_file("uchat-client/src/gui/resources/eye-close.png");

    gtk_button_set_image(GTK_BUTTON(pw_button), hide_pw_image);
    gtk_entry_set_visibility(GTK_ENTRY(pw_entry), TRUE);
  }
}

void registration_submit(GtkWidget *registration_button, t_form_data *data) {
  GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));
  char *username = (char *)gtk_entry_get_text(GTK_ENTRY(data->username));
  char *password = (char *)gtk_entry_get_text(GTK_ENTRY(data->password));
  char *repassword = (char *)gtk_entry_get_text(GTK_ENTRY(data->repassword));

  if (gtk_style_context_has_class(gtk_widget_get_style_context(data->message),
                                  "form-message-success")) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(data->message),
                                   "form-message-success");
  }
  if (check_form_data(username, password, data->message) == 1) {
    if (strcmp(password, repassword) != 0)
      gtk_label_set_text(GTK_LABEL(data->message),
                         "Passwords must be the same");
    else {
      // send JSON REGISTER
      char *json_str = build_json_register(username, password);
      // g_print("Sock: %i", data->sock);
      send(data->sock, json_str, strlen(json_str), 0);
      g_free(json_str);
      // g_print("Registration sended to server:\nUsername: %s\nPassword: "
      //"%s\nRepassword: %s\n",
      // username, password, repassword);
      gtk_entry_set_text(GTK_ENTRY(data->username), "");
      gtk_entry_set_text(GTK_ENTRY(data->password), "");
      gtk_entry_set_text(GTK_ENTRY(data->repassword), "");
      gtk_label_set_text(GTK_LABEL(data->message), "");
      gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
      gtk_entry_set_visibility(GTK_ENTRY(data->repassword), TRUE);
      change_password_visibility(data->pw_button, data->password);
      change_password_visibility(data->repw_button, data->repassword);
    }
  }
}

void login_submit(GtkWidget *login_button, t_form_data *data) {
  GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(data->form));
  char *username = (char *)gtk_entry_get_text(GTK_ENTRY(data->username));
  char *password = (char *)gtk_entry_get_text(GTK_ENTRY(data->password));

  if (gtk_style_context_has_class(gtk_widget_get_style_context(data->message),
                                  "form-message-success")) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(data->message),
                                   "form-message-success");
  }
  if (check_form_data(username, password, data->message) == 1) {
    // send JSON LOGIN
    char serial_number[64] = {0};
    get_serial_number(serial_number, sizeof(serial_number));
    char *json_str = build_json_login(username, password, serial_number);
    // g_print("Sock: %i", data->sock);
    send(data->sock, json_str, strlen(json_str), 0);
    g_free(json_str);
    // g_print("Login sended to server:\nUsername: %s\nPassword: %s\n",
    // username,
    // password);
    gtk_entry_set_text(GTK_ENTRY(data->username), "");
    gtk_entry_set_text(GTK_ENTRY(data->password), "");
    gtk_label_set_text(GTK_LABEL(data->message), "");
    gtk_entry_set_visibility(GTK_ENTRY(data->password), TRUE);
    change_password_visibility(data->pw_button, data->password);
  }
}

int check_username(char *username, GtkWidget *message) {
  int username_len = strlen(username);

  // Check if username is provided
  if (username_len == 0) {
    gtk_label_set_text(GTK_LABEL(message), "Username is required");
    return 0;
  } else if (username_len < 2 || username_len > 20) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Username must contain 2-20 symbols");
    return 0;
  }

  // Check if username contains only lowercase letters
  for (int i = 0; i < username_len; i++) {
    if (!islower(username[i])) {
      gtk_label_set_text(GTK_LABEL(message),
                         "Username must only be lowercase letters");
      return 0;
    }
  }
  return 1;
}

int check_password(char *password, GtkWidget *message) {
  int password_len = password != NULL ? strlen(password) : -1;
  if (password_len == 0) {
    gtk_label_set_text(GTK_LABEL(message), "Password is required");
    return 0;
  } else if (password_len < 8 || password_len > 20) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Password must contain 8-20 symbols");
    return 0;
  }
  // check for password reliability here

  int has_upper = 0, has_lower = 0, has_digit = 0;

  for (int i = 0; i < password_len; i++) {
    if (isupper(password[i]))
      has_upper = 1;
    if (islower(password[i]))
      has_lower = 1;
    if (isdigit(password[i]))
      has_digit = 1;

    // Break early if all conditions are met
    if (has_upper && has_lower && has_digit)
      break;
  }
  if (!has_upper) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Password must contain at least 1 uppercase letter");
    return 0;
  }
  if (!has_lower) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Password must contain at least 1 lowercase letter");
    return 0;
  }

  if (!has_digit) {
    gtk_label_set_text(GTK_LABEL(message),
                       "Password must contain at least 1 number");
    return 0;
  }

  return 1;
}

int check_form_data(char *username, char *password, GtkWidget *message) {
  // Check if the username is valid
  if (check_username(username, message) == 0) {
    return 0; // Username validation failed, no need to check password
  }
  // If password is provided, check if it is valid
  if (password != NULL) {
    if (check_password(password, message) == 0) {
      return 0;
    }
  }
  return 1;
}
