#include <uchat.h>

void create_update_failed_window() {
  if (retry_window)
    return; // Avoid duplicate windows

  retry_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(retry_window), "Connection Failed");
  gtk_window_set_default_size(GTK_WINDOW(retry_window), 400, 300);
  gtk_window_set_position(GTK_WINDOW(retry_window), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable(GTK_WINDOW(retry_window), FALSE); // Prevent resizing
  g_signal_connect(retry_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // Main container
  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  gtk_container_set_border_width(GTK_CONTAINER(container), 20); // Add padding
  gtk_container_add(GTK_CONTAINER(retry_window), container);
  gtk_widget_set_halign(container, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(container,
                        GTK_ALIGN_CENTER); // Center the container itself
  gtk_style_context_add_class(gtk_widget_get_style_context(container),
                              "uf-container");

  // Label for "Connection Failed"
  GtkWidget *container_label = gtk_label_new("Connection failed");
  gtk_box_pack_start(GTK_BOX(container), container_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(container_label),
                              "uf-container-label");

  // Spinner
  GtkWidget *retry_spinner = gtk_spinner_new();
  gtk_box_pack_start(GTK_BOX(container), retry_spinner, FALSE, FALSE, 0);
  gtk_widget_set_size_request(GTK_WIDGET(retry_spinner), 50, 50);
  gtk_spinner_start(GTK_SPINNER(retry_spinner));
  gtk_style_context_add_class(gtk_widget_get_style_context(retry_spinner),
                              "uf-retry-spinner");

  // Countdown label
  retry_label = gtk_label_new("Retrying in 10 sec...");
  gtk_box_pack_start(GTK_BOX(container), retry_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(retry_label),
                              "uf-retry-label");

  // Retry button
  GtkWidget *retry_button = gtk_button_new_with_label("Retry now");
  gtk_box_pack_start(GTK_BOX(container), retry_button, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(retry_button),
                              "uf-retry-button");
  g_signal_connect(retry_button, "clicked", G_CALLBACK(on_retry_clicked), NULL);

  // Load the CSS file
  load_css("uchat-client/src/gui/login_registration.css");

  gtk_widget_show_all(retry_window);

  // Start the retry countdown
  retry_timeout_id = g_timeout_add_seconds(1, retry_connection, NULL);
  gtk_main();
}

void on_retry_clicked(GtkButton *button, gpointer data) {
  if (retry_timeout_id > 0) {
    g_source_remove(retry_timeout_id); // Stop the current countdown
    retry_timeout_id = 0;              // Reset to prevent double removal
  }
  retry_timeout = 0;      // Set to zero to trigger immediate retry
  attempt_reconnection(); // Immediately attempt reconnection
}

gboolean retry_connection(gpointer data) {
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Retrying in %d seconds...", retry_timeout);
  gtk_label_set_text(GTK_LABEL(retry_label), buffer);

  if (retry_timeout == 0) {
    attempt_reconnection(); // Attempt reconnection when countdown reaches zero
    return FALSE;           // Stop the timeout if reconnection is attempted
  } else {
    retry_timeout--;
  }
  return TRUE; // Continue countdown
}