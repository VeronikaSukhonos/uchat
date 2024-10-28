#include <uchat.h>

void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data) {
  GtkWidget *form_name_label;
  GtkWidget *username_label;
  GtkWidget *password_label;
  GtkWidget *login_button;
  GtkWidget *registration_link_label;
  GtkWidget *registration_link_button;

  login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK(pages), login, "login");
  gtk_style_context_add_class(gtk_widget_get_style_context(login), "log-reg");

  data->form = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(login), data->form, FALSE, FALSE, 50);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->form), "form");
  gtk_widget_set_halign(data->form, GTK_ALIGN_CENTER);

  form_name_label = gtk_label_new("Login");
  gtk_style_context_add_class(gtk_widget_get_style_context(form_name_label),
                              "form-name-label");
  gtk_grid_attach(GTK_GRID(data->form), form_name_label, 0, 0, 2, 1);

  username_label = gtk_label_new("Username");
  data->username = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(username_label),
                              "form-label");
  gtk_style_context_add_class(gtk_widget_get_style_context(data->username),
                              "form-entry");
  gtk_grid_attach(GTK_GRID(data->form), username_label, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(data->form), data->username, 0, 2, 2, 1);
  gtk_widget_set_halign(username_label, GTK_ALIGN_START);

  password_label = gtk_label_new("Password");
  data->password = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(password_label),
                              "form-label");
  gtk_style_context_add_class(gtk_widget_get_style_context(data->password),
                              "form-entry");
  gtk_grid_attach(GTK_GRID(data->form), password_label, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(data->form), data->password, 0, 4, 2, 1);
  gtk_widget_set_halign(password_label, GTK_ALIGN_START);

  data->message = gtk_label_new("");
  gtk_grid_attach(GTK_GRID(data->form), data->message, 0, 5, 2, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->message),
                              "form-message");

  login_button = gtk_button_new_with_label("Log In");
  gtk_grid_attach(GTK_GRID(data->form), login_button, 0, 6, 2, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(login_button),
                              "form-button");
  g_signal_connect(login_button, "clicked", G_CALLBACK(login_submit), data);

  registration_link_label = gtk_label_new("Don't have an account? ");
  registration_link_button = gtk_button_new_with_label("Register");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(registration_link_label), "form-link-label");
  gtk_style_context_add_class(
      gtk_widget_get_style_context(registration_link_button), "form-link");
  gtk_grid_attach(GTK_GRID(data->form), registration_link_label, 0, 7, 1, 1);
  gtk_grid_attach(GTK_GRID(data->form), registration_link_button, 1, 7, 1, 1);
  g_signal_connect(registration_link_button, "clicked",
                   G_CALLBACK(show_registration), data);
}