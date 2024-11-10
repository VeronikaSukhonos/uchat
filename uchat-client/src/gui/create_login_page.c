#include <uchat.h>

void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data) {
  login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK(pages), login, "login");
  gtk_style_context_add_class(gtk_widget_get_style_context(login), "log-reg");

  data->form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(login), data->form, TRUE, FALSE, 0);
  gtk_widget_set_size_request(GTK_WIDGET(data->form), 450, -1);
  gtk_widget_set_halign(data->form, GTK_ALIGN_CENTER);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->form), "form");

  GtkWidget *form_name_label = gtk_label_new("Login");
  gtk_box_pack_start(GTK_BOX(data->form), form_name_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(form_name_label),
                              "form-name-label");

  GtkWidget *username_label = gtk_label_new("Username");
  gtk_box_pack_start(GTK_BOX(data->form), username_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(username_label, GTK_ALIGN_START);
  gtk_style_context_add_class(gtk_widget_get_style_context(username_label),
                              "form-label");

  data->username = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(data->form), data->username, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->username),
                              "form-entry");

  GtkWidget *password_label = gtk_label_new("Password");
  gtk_box_pack_start(GTK_BOX(data->form), password_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(password_label, GTK_ALIGN_START);
  gtk_style_context_add_class(gtk_widget_get_style_context(password_label),
                              "form-label");

  GtkWidget *pw_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(data->form), pw_container, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(pw_container),
                              "form-pw-container");

  data->password = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(pw_container), data->password, TRUE, TRUE, 0);
  gtk_entry_set_visibility(GTK_ENTRY(data->password), FALSE);
  g_object_set(data->password, "caps-lock-warning", FALSE, NULL);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->password),
                              "form-pw-entry");
  g_signal_connect(data->password, "focus-in-event",
                   G_CALLBACK(change_password_focus), pw_container);
  g_signal_connect(data->password, "focus-out-event",
                   G_CALLBACK(change_password_focus), pw_container);

  data->pw_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(pw_container), data->pw_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->pw_button),
                              "form-pw-button");

  GtkWidget *show_pw_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/eye-open.png");
  gtk_button_set_image(GTK_BUTTON(data->pw_button), show_pw_image);
  g_signal_connect(data->pw_button, "clicked",
                   G_CALLBACK(change_password_visibility), data->password);

  data->message = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(data->form), data->message, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->message),
                              "form-message");

  GtkWidget *login_button = gtk_button_new_with_label("Log In");
  gtk_box_pack_start(GTK_BOX(data->form), login_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(login_button),
                              "form-button");
  g_signal_connect(login_button, "clicked", G_CALLBACK(login_submit), data);

  GtkWidget *link_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(data->form), link_container, FALSE, FALSE, 0);
  gtk_widget_set_halign(link_container, GTK_ALIGN_CENTER);

  GtkWidget *registration_link_label = gtk_label_new("Don't have an account? ");
  gtk_box_pack_start(GTK_BOX(link_container), registration_link_label, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(registration_link_label), "form-link-label");

  GtkWidget *registration_link_button = gtk_button_new_with_label("Register");
  gtk_box_pack_start(GTK_BOX(link_container), registration_link_button, FALSE,
                     FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(registration_link_button), "form-link");
  g_signal_connect(registration_link_button, "clicked",
                   G_CALLBACK(show_registration), data);
}
