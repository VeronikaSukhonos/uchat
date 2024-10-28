#include <uchat.h>

void create_chats_page(GtkWidget *pages, GtkWidget *chats) {
  chats = gtk_grid_new();
  gtk_stack_add_named(GTK_STACK(pages), chats, "chats");
}