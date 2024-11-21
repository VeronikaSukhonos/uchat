#include <uchat.h>

//play a voice message(temporarily)
void play_voice(GtkWidget *button, gpointer data) {
    g_print("The message is being played\n");
}

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

gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        if ((event->state & GDK_SHIFT_MASK) != 0) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
            gtk_text_buffer_insert_at_cursor(buffer, "\n", -1); 
            return TRUE; 
        } else {
            send_message_f(widget, user_data);
            return TRUE; 
        }
    }

    return FALSE; 
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

MessageNode *create_message_node(t_main_page_data *main_page, ContentType message_type, int chat_id) {
  MessageNode *temp_node;
  int name_i = 0;
  if ((*main_page).messages == NULL) {
    (*main_page).messages = malloc(sizeof(MessageNode));
    temp_node = (*main_page).messages;
  }
  else {
    ++name_i;
    temp_node = (*main_page).messages;
    while (temp_node->next != NULL) {
      temp_node = temp_node->next;
      ++name_i;
    }
    temp_node->next = malloc(sizeof(MessageNode));
    temp_node = temp_node->next;
  }
  temp_node->next = NULL;

  (*temp_node).message.chat_id = chat_id;
  (*temp_node).message.date = time(NULL);
  (*temp_node).message.content_type = message_type;
  (*temp_node).message.status = NEW;

  if (message_type == TEXT) {
    char temp_message_text[13];
    strcpy(temp_message_text, "New message");
    temp_message_text[11] = name_i % 10 + '0';
    temp_message_text[12] = '\0';
    int last_message_length = strlen(temp_message_text);
    strncpy((*temp_node).message.content, temp_message_text, last_message_length);
    (*temp_node).message.content[last_message_length] = '\0';
  }
  else {
    // set voise path
  }
  (*temp_node).message.button = NULL;

  return temp_node;
}

void create_message_button(t_main_page_data *main_page, MessageNode *temp_node) {
  t_chat_node *chat_with_new_message;
  for (chat_with_new_message = (*main_page).chats; chat_with_new_message->chat.id != (*temp_node).message.chat_id; chat_with_new_message = chat_with_new_message->next);
  (*temp_node).message.button = gtk_button_new();
  gtk_box_pack_end(GTK_BOX(chat_with_new_message->chat.box), (*temp_node).message.button,
                     FALSE, FALSE, 0);
  gtk_box_reorder_child(GTK_BOX(chat_with_new_message->chat.box), (*temp_node).message.button, 0);
  //if (sender is current user) {
    gtk_widget_set_halign((*temp_node).message.button, GTK_ALIGN_END);
    gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.button),
                                "outgoing-messages");
  //}
  //else {
  //  gtk_widget_set_halign((*temp_node).message.button, GTK_ALIGN_START);
  //  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.button),
  //                              "ingoing-messages");
  //}

  GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER((*temp_node).message.button), main_box);

  if ((*temp_node).message.content_type == TEXT) {
    (*temp_node).message.message_label = gtk_label_new((*temp_node).message.content);
    gtk_box_pack_start(GTK_BOX(main_box), (*temp_node).message.message_label, FALSE, FALSE, 0);
  }
  else {
    GtkWidget *play_button = gtk_image_new_from_file("uchat-client/src/gui/resources/play_button.png");
    (*temp_node).message.voice_message_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON((*temp_node).message.voice_message_button), play_button);
    gtk_box_pack_start(GTK_BOX(main_box), (*temp_node).message.voice_message_button, FALSE, FALSE, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.voice_message_button),
                                "play-button");
    g_signal_connect((*temp_node).message.voice_message_button, "clicked",
                   G_CALLBACK(play_voice), NULL);
  }

  GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_pack_start(GTK_BOX(main_box), bottom_box, TRUE, TRUE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(bottom_box),
                              "chat-button-bottom-box");

  (*temp_node).message.seen_label = gtk_label_new("");
  gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message.seen_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.seen_label),
                              "small-inscriptions");
  gtk_widget_set_halign((*temp_node).message.seen_label, GTK_ALIGN_END);

  (*temp_node).message.time_label = gtk_label_new("00:00");
  gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message.time_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.time_label),
                              "small-inscriptions");
  gtk_widget_set_halign((*temp_node).message.time_label, GTK_ALIGN_END);

  (*temp_node).message.changed_label = gtk_label_new("");
  gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message.changed_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context((*temp_node).message.changed_label),
                              "small-inscriptions");
  gtk_widget_set_halign((*temp_node).message.changed_label, GTK_ALIGN_END);

  //if (strcmp(unread, "") == 0)
  //  gtk_style_context_remove_class(gtk_widget_get_style_context((*main_page).opened_chat->button),
  //                                 "chat-button-unread-border");
  //else
  //  gtk_style_context_add_class(gtk_widget_get_style_context((*main_page).opened_chat->button),
  //                              "chat-button-unread-border");

  gtk_widget_set_visible((*temp_node).message.button, 1);
  gtk_widget_set_visible(main_box, 1);
  gtk_widget_set_visible(bottom_box, 1);
  if ((*temp_node).message.content_type == TEXT)
    gtk_widget_set_visible((*temp_node).message.message_label, 1);
  else
    gtk_widget_set_visible((*temp_node).message.voice_message_button, 1);
  gtk_widget_set_visible((*temp_node).message.changed_label, 1);
  gtk_widget_set_visible((*temp_node).message.time_label, 1);
  gtk_widget_set_visible((*temp_node).message.seen_label, 1);
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
    MessageNode *new_message = create_message_node(main_page, TEXT, chat_id);
    create_message_button(main_page, new_message);
  } else {
    g_print("Cannot send an empty message.\n");
  }
  gtk_text_buffer_set_text(message_buffer, "", -1);
}
