#include <uchat.h>

void show_smile_menu(GtkWidget *smile_button, GtkWidget *smile_window) {
  GtkWidget *smile_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/smile-close.png");
  gtk_button_set_image(GTK_BUTTON(smile_button), smile_image);
  gtk_popover_popup(GTK_POPOVER(smile_window));
  gtk_widget_show_all(smile_window);
}
void hide_smile_menu(GtkWidget *smile_window, GtkWidget *smile_button) {
  GtkWidget *smile_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/smile-open.png");
  gtk_button_set_image(GTK_BUTTON(smile_button), smile_image);
}
void insert_emoji_into_text(GtkWidget *emoji_button,
                            t_main_page_data *main_page) {
  GtkTextBuffer *message_buffer = GTK_TEXT_BUFFER(main_page->message_buffer);
  const gchar *emoji = gtk_button_get_label(GTK_BUTTON(emoji_button));
  GtkTextIter cursor;
  gtk_text_buffer_get_iter_at_mark(message_buffer, &cursor,
                                   gtk_text_buffer_get_insert(message_buffer));
  gtk_text_buffer_insert(message_buffer, &cursor, emoji, -1);
}

void change_button_hover_image(GtkWidget *send_button) {
  GdkPixbuf *default_pixbuf = gdk_pixbuf_new_from_file(
      "uchat-client/src/gui/resources/send-button-purple.png", NULL);
  GdkPixbuf *hover_pixbuf = gdk_pixbuf_new_from_file(
      "uchat-client/src/gui/resources/send-button.png", NULL);

  // Change image when hovering
  g_signal_connect(send_button, "enter-notify-event",
                   G_CALLBACK(on_button_hover), hover_pixbuf);
  g_signal_connect(send_button, "leave-notify-event",
                   G_CALLBACK(on_button_leave), default_pixbuf);
}

gboolean on_button_hover(GtkWidget *send_button, GdkEvent *event,
                         gpointer user_data) {
  GdkPixbuf *hover_pixbuf = (GdkPixbuf *)user_data;
  GtkWidget *image = gtk_button_get_image(GTK_BUTTON(send_button));
  gtk_image_set_from_pixbuf(GTK_IMAGE(image), hover_pixbuf);
  return FALSE;
}

gboolean on_button_leave(GtkWidget *send_button, GdkEvent *event,
                         gpointer user_data) {
  GdkPixbuf *default_pixbuf = (GdkPixbuf *)user_data;
  GtkWidget *image = gtk_button_get_image(GTK_BUTTON(send_button));
  gtk_image_set_from_pixbuf(GTK_IMAGE(image), default_pixbuf);
  return FALSE;
}

void check_message_entry_height(GtkTextBuffer *message_buffer,
                                GtkWidget *message_entry) {
  GtkWidget *message_scroll = gtk_widget_get_parent(message_entry);
  GtkTextIter start, end;
  int height;

  gtk_text_buffer_get_bounds(message_buffer, &start, &end);
  if (gtk_text_iter_equal(&start, &end) == TRUE)
    height = 30;
  else
    gtk_widget_get_preferred_height(message_entry, &height, NULL);
  if (height > 143) {
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_widget_set_size_request(message_scroll, -1, 144);
  } else {
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(message_scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_widget_set_size_request(message_scroll, -1, height);
  }
}

const gchar *message_trim(const gchar *message) {
  int start = -1;
  int finish = 0;

  if (message == NULL)
    return NULL;
  for (int i = 0; message[i] != '\0'; i++) {
    if (!isspace(message[i])) {
      finish = i;
      if (start == -1)
        start = i;
    }
  }
  if (start == -1)
    return "";
  return g_strndup(message + start, finish - start + 1);
}

void send_message_to_server(int chat_id, const gchar *message) {
  cJSON *json_message = cJSON_CreateObject();
  cJSON_AddStringToObject(json_message, "action", "SEND_MESSAGE_TO_CHAT");
  cJSON_AddNumberToObject(json_message, "chat_id", chat_id);
  cJSON_AddStringToObject(json_message, "message", message);

  char *json_str = cJSON_Print(json_message);
  g_print("Sending message to server: %s\n", json_str);
  send(sock, json_str, strlen(json_str), 0);

  cJSON_Delete(json_message);
  free(json_str);
  g_free((gpointer)message);
}

void send_message_f(GtkWidget *widget, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  GtkTextBuffer *message_buffer = GTK_TEXT_BUFFER(main_page->message_buffer);
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds(message_buffer, &start, &end);
  const gchar *message_text = message_trim(
      gtk_text_buffer_get_text(message_buffer, &start, &end, FALSE));
  // after using t_chat_form_data->username appears segmentation fault

  int chat_id = main_page->opened_chat->id;

  if (message_text != NULL && g_strcmp0(message_text, "") != 0) {
    send_message_to_server(chat_id, message_text);
  } else {
    g_print("Cannot send an empty message.\n");
  }
  gtk_text_buffer_set_text(message_buffer, "", -1);
}
