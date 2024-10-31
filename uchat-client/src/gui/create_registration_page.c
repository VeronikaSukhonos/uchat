#include <uchat.h>

void create_registration_page(GtkWidget *pages, GtkWidget *registration,
                              t_form_data *data) {
	registration = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_named(GTK_STACK(pages), registration, "registration");
	gtk_style_context_add_class(gtk_widget_get_style_context(registration),
								"log-reg");

	data->form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(registration), data->form, TRUE, FALSE, 0);
	gtk_widget_set_size_request(GTK_WIDGET(data->form), 450, -1);
	gtk_widget_set_halign(data->form, GTK_ALIGN_CENTER);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->form), "form");

	GtkWidget *form_name_label = gtk_label_new("Registration");
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

	data->password = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(data->form), data->password, FALSE, FALSE, 0);
	gtk_entry_set_visibility(GTK_ENTRY(data->password), FALSE);
	g_object_set(data->password, "caps-lock-warning", FALSE, NULL);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->password),
								"form-entry");

	GtkWidget *repassword_label = gtk_label_new("Repeat password");
	gtk_box_pack_start(GTK_BOX(data->form), repassword_label, FALSE, FALSE, 0);
	gtk_widget_set_halign(repassword_label, GTK_ALIGN_START);
	gtk_style_context_add_class(gtk_widget_get_style_context(repassword_label),
								"form-label");

	data->repassword = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(data->form), data->repassword, FALSE, FALSE, 0);
	gtk_entry_set_visibility(GTK_ENTRY(data->repassword), FALSE);
	g_object_set(data->repassword, "caps-lock-warning", FALSE, NULL);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->repassword),
								"form-entry");

	data->message = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(data->form), data->message, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->message),
								"form-message");

	GtkWidget *registration_button = gtk_button_new_with_label("Register");
	gtk_box_pack_start(GTK_BOX(data->form), registration_button, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(registration_button),
								"form-button");
	g_signal_connect(registration_button, "clicked",
					 G_CALLBACK(registration_submit), data);

	GtkWidget *link_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(data->form), link_container, FALSE, FALSE, 0);
	gtk_widget_set_halign(link_container, GTK_ALIGN_CENTER);

	GtkWidget *login_link_label = gtk_label_new("Already have an account? ");
	gtk_box_pack_start(GTK_BOX(link_container), login_link_label, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(login_link_label),
								"form-link-label");

	GtkWidget *login_link_button = gtk_button_new_with_label("Log in");
	gtk_box_pack_start(GTK_BOX(link_container), login_link_button, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(login_link_button),
								"form-link");
	g_signal_connect(login_link_button, "clicked", G_CALLBACK(show_login), data);
}

