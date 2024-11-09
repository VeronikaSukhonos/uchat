#include <gtk/gtk.h>

void load_css(const gchar *file) {
	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(
		gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_css_provider_load_from_path(provider, file, NULL);
	g_object_unref(provider);
}

void create_update_failed_page(GtkWidget *pages, GtkWidget *update_failed) {
	update_failed = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_stack_add_named(GTK_STACK(pages), update_failed, "update_failed");
	gtk_style_context_add_class(gtk_widget_get_style_context(update_failed),
								"uf");

	GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(update_failed), container, TRUE, FALSE, 0);
	gtk_widget_set_size_request(GTK_WIDGET(container), 450, -1);
	gtk_widget_set_halign(container, GTK_ALIGN_CENTER);
	gtk_style_context_add_class(gtk_widget_get_style_context(container),
								"uf-container");

	GtkWidget *container_label = gtk_label_new("Update failed");
	gtk_box_pack_start(GTK_BOX(container), container_label, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(container_label),
								"uf-container-label");

	GtkWidget *retry_spinner = gtk_spinner_new();
	gtk_box_pack_start(GTK_BOX(container), retry_spinner, FALSE, FALSE, 0);
	gtk_widget_set_size_request(GTK_WIDGET(retry_spinner), 120, 120);
	gtk_spinner_start(GTK_SPINNER(retry_spinner));

	GtkWidget *retry_label = gtk_label_new("Retrying in 10 sec...");
	gtk_box_pack_start(GTK_BOX(container), retry_label, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(retry_label),
								"uf-retry-label");

	GtkWidget *retry_button = gtk_button_new_with_label("Retry now");
	gtk_box_pack_start(GTK_BOX(container), retry_button, FALSE, FALSE, 0);
	gtk_style_context_add_class(gtk_widget_get_style_context(retry_button),
								"uf-retry-button");
	//g_signal_connect(retry_button, "clicked", G_CALLBACK(some_function_to_retry), NULL);

	//gtk_spinner_stop(GTK_SPINNER(retry_spinner));
}

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), "Green Chat");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1280, 720);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	GtkWidget *pages = gtk_stack_new();
	gtk_container_add(GTK_CONTAINER(main_window), pages);

	GtkWidget *update_failed;
	create_update_failed_page(pages, update_failed);

	// add to real css some marked classes from this copy
	load_css("login_registration.css");

	gtk_widget_show_all(main_window);
	gtk_stack_set_visible_child_name(GTK_STACK(pages), "update_failed");
	gtk_stack_set_transition_type(GTK_STACK(pages), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
	gtk_main();
}

