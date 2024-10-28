// sudo apt install libgtk-3-dev
// clang gui.c -o gui `pkg-config --cflags --libs gtk+-3.0`

#include <gtk/gtk.h>
#include <string.h>

typedef struct s_form_data {
	GtkWidget *form;
	GtkWidget *username;
	GtkWidget *password;
	GtkWidget *repassword;
	GtkWidget *message;
}			   t_form_data;


void load_css(const gchar *file) {
	GtkCssProvider *provider = gtk_css_provider_new();

	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
											  GTK_STYLE_PROVIDER(provider),
											  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_css_provider_load_from_path(provider, file, NULL);

	g_object_unref(provider);
}

void create_registration_page(GtkWidget *pages, GtkWidget *registration, t_form_data *data);
void create_login_page(GtkWidget *pages, GtkWidget *login, t_form_data *data);
void create_chats_page(GtkWidget *pages, GtkWidget *chats);

void show_registration(GtkWidget *registration_link_button, gpointer pages);
void show_login(GtkWidget *login_link_button, gpointer pages);

void registration_submit(GtkWidget *registration_button, gpointer data);
void login_submit(GtkWidget *login_button, gpointer data);

int check_form_data(char *username, char *password, GtkWidget *message);


int main(int argc, char *argv[]) { // server IP address and port
	GtkWidget *main_window, *pages;
	GtkWidget *registration, *login, *chats;
	t_form_data registration_data, login_data;

	// all that stuff with connecting to server?

	gtk_init(&argc, &argv);

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), "Green Chat");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 450);
	g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	pages = gtk_stack_new();
	gtk_container_add(GTK_CONTAINER(main_window), pages);

	create_registration_page(pages, registration, &registration_data);
	create_login_page(pages, login, &login_data);
	create_chats_page(pages, chats);

	load_css("login_registration.css");

	gtk_widget_show_all(main_window);
	// if not logined
	gtk_stack_set_visible_child_name(GTK_STACK(pages), "login");
	// else
	// gtk_stack_set_visible_child_name(GTK_STACK(pages), "chats");
	gtk_stack_set_transition_type(GTK_STACK(pages), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
	gtk_main();
}


void create_registration_page(GtkWidget *pages, GtkWidget *registration, t_form_data *data) {
	GtkWidget *form_name_label;
	GtkWidget *username_label;
	GtkWidget *password_label;
	GtkWidget *repassword_label;
	GtkWidget *registration_button;
	GtkWidget *login_link_label;
	GtkWidget *login_link_button;

	registration = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_named(GTK_STACK(pages), registration, "registration");
	gtk_style_context_add_class(gtk_widget_get_style_context(registration), "log-reg");

	data->form = gtk_grid_new();
	gtk_box_pack_start(GTK_BOX(registration), data->form, FALSE, FALSE, 50);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->form), "form");
	gtk_widget_set_halign(data->form, GTK_ALIGN_CENTER);

	form_name_label = gtk_label_new("Registration");
	gtk_style_context_add_class(gtk_widget_get_style_context(form_name_label), "form-name-label");
	gtk_grid_attach(GTK_GRID(data->form), form_name_label, 0, 0, 2, 1);

	username_label = gtk_label_new("Username");
	data->username = gtk_entry_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(username_label), "form-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(data->username), "form-entry");
	gtk_grid_attach(GTK_GRID(data->form), username_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), data->username, 0, 2, 2, 1);
	gtk_widget_set_halign(username_label, GTK_ALIGN_START);

	password_label = gtk_label_new("Password");
	data->password = gtk_entry_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(password_label), "form-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(data->password), "form-entry");
	gtk_grid_attach(GTK_GRID(data->form), password_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), data->password, 0, 4, 2, 1);
	gtk_widget_set_halign(password_label, GTK_ALIGN_START);

	repassword_label = gtk_label_new("Repeat password");
	data->repassword = gtk_entry_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(repassword_label), "form-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(data->repassword), "form-entry");
	gtk_grid_attach(GTK_GRID(data->form), repassword_label, 0, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), data->repassword, 0, 6, 2, 1);
	gtk_widget_set_halign(repassword_label, GTK_ALIGN_START);

	data->message = gtk_label_new("");
	gtk_grid_attach(GTK_GRID(data->form), data->message, 0, 7, 2, 1);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->message), "form-message");

	registration_button = gtk_button_new_with_label("Register");
	gtk_grid_attach(GTK_GRID(data->form), registration_button, 0, 8, 2, 1);
	gtk_style_context_add_class(gtk_widget_get_style_context(registration_button), "form-button");
	g_signal_connect(registration_button, "clicked", G_CALLBACK(registration_submit), data);

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
	gtk_style_context_add_class(gtk_widget_get_style_context(form_name_label), "form-name-label");
	gtk_grid_attach(GTK_GRID(data->form), form_name_label, 0, 0, 2, 1);

	username_label = gtk_label_new("Username");
	data->username = gtk_entry_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(username_label), "form-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(data->username), "form-entry");
	gtk_grid_attach(GTK_GRID(data->form), username_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), data->username, 0, 2, 2, 1);
	gtk_widget_set_halign(username_label, GTK_ALIGN_START);

	password_label = gtk_label_new("Password");
	data->password = gtk_entry_new();
	gtk_style_context_add_class(gtk_widget_get_style_context(password_label), "form-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(data->password), "form-entry");
	gtk_grid_attach(GTK_GRID(data->form), password_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), data->password, 0, 4, 2, 1);
	gtk_widget_set_halign(password_label, GTK_ALIGN_START);

	data->message = gtk_label_new("");
	gtk_grid_attach(GTK_GRID(data->form), data->message, 0, 5, 2, 1);
	gtk_style_context_add_class(gtk_widget_get_style_context(data->message), "form-message");

	login_button = gtk_button_new_with_label("Log In");
	gtk_grid_attach(GTK_GRID(data->form), login_button, 0, 6, 2, 1);
	gtk_style_context_add_class(gtk_widget_get_style_context(login_button), "form-button");
	g_signal_connect(login_button, "clicked", G_CALLBACK(login_submit), data);

	registration_link_label = gtk_label_new("Don't have an account? ");
	registration_link_button = gtk_button_new_with_label("Register");
	gtk_style_context_add_class(gtk_widget_get_style_context(registration_link_label),
															 "form-link-label");
	gtk_style_context_add_class(gtk_widget_get_style_context(registration_link_button),
															 "form-link");
	gtk_grid_attach(GTK_GRID(data->form), registration_link_label, 0, 7, 1, 1);
	gtk_grid_attach(GTK_GRID(data->form), registration_link_button, 1, 7, 1, 1);
	g_signal_connect(registration_link_button, "clicked", G_CALLBACK(show_registration), data);
}

void create_chats_page(GtkWidget *pages, GtkWidget *chats) {
	chats = gtk_grid_new();
	gtk_stack_add_named(GTK_STACK(pages), chats, "chats");
}

void show_login(GtkWidget *login_link_button, gpointer data) {
	t_form_data *user_data = (t_form_data *)data;
	GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));

	gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
	gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
	gtk_entry_set_text(GTK_ENTRY(user_data->repassword), "");
	gtk_label_set_text(GTK_LABEL(user_data->message), "");
	gtk_stack_set_visible_child_name(GTK_STACK(pages), "login");
}

void show_registration(GtkWidget *registration_link_button, gpointer data) {
	t_form_data *user_data = (t_form_data *)data;
	GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));

	gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
	gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
	gtk_label_set_text(GTK_LABEL(user_data->message), "");
	gtk_stack_set_visible_child_name(GTK_STACK(pages), "registration");
}

void registration_submit(GtkWidget *registration_button, gpointer data) {
	t_form_data *user_data = (t_form_data *)data;
	GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));
    char *username = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->username));
    char *password = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->password));
    char *repassword = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->repassword));

	if (check_form_data(username, password, user_data->message) == 1) {
		if (strcmp(password, repassword) != 0)
			gtk_label_set_text(GTK_LABEL(user_data->message), "Passwords must be the same");
		else {
			g_print("Registration sended to server:\nUsername: %s\nPassword: %s\nRepassword: %s\n",
					username, password, repassword);
			gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
			gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
			gtk_entry_set_text(GTK_ENTRY(user_data->repassword), "");
			gtk_label_set_text(GTK_LABEL(user_data->message), "");
			gtk_stack_set_visible_child_name(GTK_STACK(pages), "chats");
		}
	}
}

void login_submit(GtkWidget *login_button, gpointer data) {
	t_form_data *user_data = (t_form_data *)data;
	GtkWidget *pages = gtk_widget_get_parent(gtk_widget_get_parent(user_data->form));
    char *username = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->username));
    char *password = (char *)gtk_entry_get_text(GTK_ENTRY(user_data->password));

	if (check_form_data(username, password, user_data->message) == 1) {
		g_print("Login sended to server:\nUsername: %s\nPassword: %s\n",
				username, password);
		gtk_entry_set_text(GTK_ENTRY(user_data->username), "");
		gtk_entry_set_text(GTK_ENTRY(user_data->password), "");
		gtk_label_set_text(GTK_LABEL(user_data->message), "");
	 	gtk_stack_set_visible_child_name(GTK_STACK(pages), "chats");
	}
}

int check_form_data(char *username, char *password, GtkWidget *message) {
	int username_len = strlen(username);
    int password_len = strlen(password);

	if (username_len == 0) {
		gtk_label_set_text(GTK_LABEL(message), "Username is required");
		return 0;
	}
	else if (username_len < 2 || username_len > 20) {
		gtk_label_set_text(GTK_LABEL(message), "Username must contain 2-20 symbols");
		return 0;
	}
	else if (password_len == 0) {
		gtk_label_set_text(GTK_LABEL(message), "Password is required");
		return 0;
	}
	else if (password_len < 8 || password_len > 20) {
		gtk_label_set_text(GTK_LABEL(message), "Password must contain 8-20 symbols");
		return 0;
	}
	return 1;
}

