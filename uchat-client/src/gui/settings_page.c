#include <uchat.h>

void change_email(GtkWidget *change_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  char *email =
      (char *)gtk_entry_get_text(GTK_ENTRY((*main_page).email_change.email));
  if (check_email(email, (*main_page).email_change.message)) {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "action", "CHANGE_EMAIL_DATA");
    cJSON_AddStringToObject(json, "username", username);
    cJSON_AddStringToObject(json, "new_email", email ? email : "");
    char *json_str = cJSON_Print(json);
    cJSON_Delete(json);
    send(main_page->sock, json_str, strlen(json_str), 0);
    // g_print("Sent: %s\n", json_str);
    g_free(json_str);
    if (send(main_page->sock, json_str, strlen(json_str), 0) == -1) {
      success_or_error_msg(main_page->email_change.message,
                           "Failed to change email. Please try again.", FALSE);
      g_free(json_str);
      return;
    }
    main_page->email_change.email = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(main_page->email_change.email), email);
    success_or_error_msg(main_page->email_change.message,
                         "Email changed successfully!", TRUE);
    switch_to_page_with_delay(main_page, "support", 1000);
  }
}

void show_email(GtkWidget *support_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  gtk_entry_set_text(GTK_ENTRY((*main_page).email_change.email), "");
  gtk_entry_set_placeholder_text(GTK_ENTRY((*main_page).email_change.email),
                                 "your_email@stud.khpi.ucode-connect.study");
  gtk_label_set_label(GTK_LABEL((*main_page).email_change.message), "");
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "email");
}
void show_support(GtkWidget *support_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  // gtk_entry_set_text(GTK_ENTRY((*main_page).support.subject_combo), "");
  gtk_entry_set_text(GTK_ENTRY((*main_page).support.support_request), "");
  gtk_label_set_label(GTK_LABEL((*main_page).support.message), "");
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "support");
}
void show_settings(GtkWidget *settings_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  set_selected_button(&(*main_page).menu_button_selected, &settings_button);

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "GET_SETTINGS_DATA");
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  // g_print("Sent: %s\n", json_str);

  g_free(json_str);
  gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
                                   "settings");
  (*main_page).opened_chat = NULL;
}
// Page Switch with delay
gboolean delayed_page_switch(gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;

  // Clear any previous message or state
  gtk_label_set_text(GTK_LABEL(main_page->support.message), "");
  gtk_label_set_text(GTK_LABEL(main_page->change_pw.message), "");
  main_page->previous_page = main_page->current_page;
  // Switch to the new page
  gtk_stack_set_visible_child_name(GTK_STACK(main_page->central_area_stack),
                                   main_page->current_page);
  main_page->current_page = main_page->current_page;

  return FALSE;
}

// Function to switch to the page after a delay
void switch_to_page_with_delay(t_main_page_data *main_page,
                               const char *page_name, guint delay_ms) {
  // Store the new page name
  main_page->previous_page = main_page->current_page;
  main_page->current_page = page_name;

  // Schedule the delayed page switch
  g_timeout_add(delay_ms, delayed_page_switch, main_page);
}
void send_support_request(GtkWidget *support_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  const gchar *support_title = gtk_combo_box_text_get_active_text(
      GTK_COMBO_BOX_TEXT(main_page->support.subject_combo));
  const gchar *support_message =
      gtk_entry_get_text(GTK_ENTRY(main_page->support.support_request));
  const gchar *email =
      gtk_entry_get_text(GTK_ENTRY(main_page->email_change.email));
  if (g_strcmp0(support_title, "") == 0 ||
      g_strcmp0(support_message, "") == 0) {
    success_or_error_msg(main_page->support.message,
                         "Please fill in all fields before submitting.", FALSE);
    return;
  }
  if (email == NULL || strcmp(email, "") == 0) {
    success_or_error_msg(main_page->support.message,
                         "Please submit your email first.", FALSE);
    switch_to_page_with_delay(main_page, "email", 1000);
    return;
  }

  cJSON *support_data = cJSON_CreateObject();
  cJSON_AddStringToObject(support_data, "action", "SUPPORT_REQUEST");
  cJSON_AddStringToObject(support_data, "username", username);
  cJSON_AddStringToObject(support_data, "email", email);
  cJSON_AddStringToObject(support_data, "title",
                          support_title ? support_title : "");
  cJSON_AddStringToObject(support_data, "message",
                          support_message ? support_message : "");

  char *json_str = cJSON_Print(support_data);
  cJSON_Delete(support_data);

  send(main_page->sock, json_str, strlen(json_str), 0);

  if (send(main_page->sock, json_str, strlen(json_str), 0) == -1) {
    success_or_error_msg(
        main_page->support.message,
        "Failed to submit the support request. Please try again.", FALSE);
    g_free(json_str);
    return;
  }

  // g_print("Sent: %s\n", json_str);
  g_free(json_str);

  gtk_combo_box_set_active(GTK_COMBO_BOX(main_page->support.subject_combo),
                           -1); // Reset combo box
  gtk_entry_set_text(GTK_ENTRY(main_page->support.support_request), "");

  success_or_error_msg(main_page->support.message,
                       "Support request submitted successfully!", TRUE);
}
void success_or_error_msg(GtkWidget *label, const char *message,
                          gboolean is_success) {
  GtkStyleContext *context = gtk_widget_get_style_context(label);
  gtk_style_context_remove_class(context, "form-message-success");
  gtk_style_context_remove_class(context, "form-message");
  if (is_success) {
    gtk_style_context_add_class(context, "form-message-success");
  } else {
    gtk_style_context_add_class(context, "form-message");
  }
  gtk_label_set_text(GTK_LABEL(label), message);
}

// Function to check if an email is valid
gboolean check_email(const char *email, GtkWidget *message_label) {
  if (!email || strlen(email) == 0) {
    success_or_error_msg(message_label, "Email cannot be empty.", FALSE);
    return FALSE;
  }

  const char *at_sign = strchr(email, '@');
  if (!at_sign) {
    success_or_error_msg(message_label, "Email must contain an '@' symbol.",
                         FALSE);
    return FALSE;
  }

  if (at_sign == email) {
    success_or_error_msg(message_label, "Email cannot start with '@'.", FALSE);
    return FALSE;
  }

  const char *domain = at_sign + 1;
  if (strlen(domain) == 0) {
    success_or_error_msg(message_label, "Email must have a domain after '@'.",
                         FALSE);
    return FALSE;
  }

  const char *dot = strchr(domain, '.');
  if (!dot) {
    success_or_error_msg(message_label,
                         "Email domain must contain a '.' symbol.", FALSE);
    return FALSE;
  }

  const char *last_dot = strrchr(domain, '.');
  if (strlen(last_dot + 1) < 2) {
    success_or_error_msg(message_label,
                         "Domain extension must have at least 2 characters.",
                         FALSE);
    return FALSE;
  }

  for (const char *c = email; *c; ++c) {
    if (!isalnum(*c) && *c != '@' && *c != '.' && *c != '-' && *c != '_') {
      success_or_error_msg(message_label, "Email contains invalid characters.",
                           FALSE);
      return FALSE;
    }
  }

  success_or_error_msg(message_label, "Email is valid.", TRUE);
  return TRUE;
}
void send_json(int socket, const char *action) {
  cJSON *json = cJSON_CreateObject();
  if (!json) {
    // g_print("Failed to create JSON object\n");
    return;
  }

  cJSON_AddStringToObject(json, "action", action);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);

  if (!json_str) {
    // g_print("Failed to create JSON string\n");
    return;
  }

  send(socket, json_str, strlen(json_str), 0);
  // g_print("Sent: %s\n", json_str);
  g_free(json_str);
}
