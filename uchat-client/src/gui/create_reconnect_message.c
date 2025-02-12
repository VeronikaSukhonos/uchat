#include "uchat.h"

void create_reconnect_message(GtkWidget *main_overlay,
                              t_reconnect_message *reconnect) {
  reconnect->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_size_request(reconnect->box, 210, 50);
  gtk_widget_set_halign(reconnect->box, GTK_ALIGN_START);
  gtk_widget_set_valign(reconnect->box, GTK_ALIGN_END);
  gtk_style_context_add_class(gtk_widget_get_style_context(reconnect->box),
                              "reconnect-message");
  gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay), reconnect->box);

  GtkWidget *reconnect_spinner = gtk_spinner_new();
  gtk_box_pack_start(GTK_BOX(reconnect->box), reconnect_spinner, FALSE, FALSE,
                     0);
  gtk_widget_set_size_request(GTK_WIDGET(reconnect_spinner), 20, 20);
  gtk_spinner_start(GTK_SPINNER(reconnect_spinner));
  gtk_style_context_add_class(gtk_widget_get_style_context(reconnect_spinner),
                              "uf-retry-spinner");

  reconnect->label = gtk_label_new("Reconnecting in 10 sec...");
  gtk_box_pack_start(GTK_BOX(reconnect->box), reconnect->label, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(reconnect->label),
                              "reconnect-message-label");

  gtk_widget_set_child_visible(reconnect->box, FALSE);
  gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(main_overlay),
                                       reconnect->box, TRUE);
}
