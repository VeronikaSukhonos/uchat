#include <uchat.h>

void create_registration_page(GtkWidget *pages, GtkWidget *registration,
                              t_form_data *data) {
  GtkWidget *form_name_label;
  GtkWidget *username_label;
  GtkWidget *password_label;
  GtkWidget *repassword_label;
  GtkWidget *registration_button;
  GtkWidget *login_link_label;
  GtkWidget *login_link_button;

  registration = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_stack_add_named(GTK_STACK(pages), registration, "registration");
  gtk_style_context_add_class(gtk_widget_get_style_context(registration),
                              "log-reg");

  data->form = gtk_grid_new();
  gtk_box_pack_start(GTK_BOX(registration), data->form, FALSE, FALSE, 50);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->form), "form");
  gtk_widget_set_halign(data->form, GTK_ALIGN_CENTER);

  form_name_label = gtk_label_new("Registration");
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

  repassword_label = gtk_label_new("Repeat password");
  data->repassword = gtk_entry_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(repassword_label),
                              "form-label");
  gtk_style_context_add_class(gtk_widget_get_style_context(data->repassword),
                              "form-entry");
  gtk_grid_attach(GTK_GRID(data->form), repassword_label, 0, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(data->form), data->repassword, 0, 6, 2, 1);
  gtk_widget_set_halign(repassword_label, GTK_ALIGN_START);

  data->message = gtk_label_new("");
  gtk_grid_attach(GTK_GRID(data->form), data->message, 0, 7, 2, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(data->message),
                              "form-message");

  registration_button = gtk_button_new_with_label("Register");
  gtk_grid_attach(GTK_GRID(data->form), registration_button, 0, 8, 2, 1);
  gtk_style_context_add_class(gtk_widget_get_style_context(registration_button),
                              "form-button");
  g_signal_connect(registration_button, "clicked",
                   G_CALLBACK(registration_submit), data);

  login_link_label = gtk_label_new("Already have an account? ");
  login_link_button = gtk_button_new_with_label("Log in");
  gtk_style_context_add_class(gtk_widget_get_style_context(login_link_label),
                              "form-link-label");
  gtk_style_context_add_class(gtk_widget_get_style_context(login_link_button),
                              "form-link");
  gtk_grid_attach(GTK_GRID(data->form), login_link_label, 0, 9, 1, 1);
  gtk_grid_attach(GTK_GRID(data->form), login_link_button, 1, 9, 1, 1);
  g_signal_connect(login_link_button, "clicked", G_CALLBACK(show_login), data);
}