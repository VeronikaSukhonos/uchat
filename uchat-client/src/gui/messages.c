#include <uchat.h>

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
  	}
  	else {
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

  char *json_string = cJSON_Print(json_message);
  g_print("Sending message to server: %s\n", json_string);

  cJSON_Delete(json_message);
  free(json_string);
  g_free((gpointer)message);
}

void send_message_f(GtkWidget *widget, gpointer data) {
  GtkTextBuffer *message_buffer = GTK_TEXT_BUFFER(data);
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds(message_buffer, &start, &end);
  const gchar *message_text = message_trim(
    gtk_text_buffer_get_text(message_buffer, &start, &end, FALSE));
  // after using t_chat_form_data->username appears segmentation fault

  int chat_id = -1;

  if (message_text != NULL && g_strcmp0(message_text, "") != 0) {
    send_message_to_server(chat_id, message_text);
  } else {
    g_print("Cannot send an empty message.\n");
  }
  gtk_text_buffer_set_text(message_buffer, "", 0);
}

