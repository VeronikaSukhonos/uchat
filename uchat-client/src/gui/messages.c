#include <gst/gst.h>
#include <time.h>
#include <uchat.h>

static pthread_t audio_thread = 0; // Track the thread
static pthread_mutex_t play_mutex =
    PTHREAD_MUTEX_INITIALIZER; // Mutex to protect thread and playback state
static GstElement *current_pipeline = NULL; // Track the current pipeline

// Function to handle dynamically linking pads
void on_pad_added(GstElement *element, GstPad *pad, GstElement *sink) {
  GstPad *sinkpad = gst_element_get_static_pad(sink, "sink");
  GstPadLinkReturn ret;
  if (!gst_pad_is_linked(sinkpad)) {
    ret = gst_pad_link(pad, sinkpad);
    if (GST_PAD_LINK_FAILED(ret)) {
      g_printerr("Failed to link decodebin pad to sink pad.\n");
    } else {
      g_print("Pad linked successfully.\n");
    }
    g_print("Pad linked dynamically.\n");
  } else {
    g_print("Pad was already linked.\n");
  }
  gst_object_unref(sinkpad);
}

// This is the function that will be run in a new thread to play the audio
void *play_audio_thread(void *data) {
  MessageNode *temp_node = (MessageNode *)data;
  char file_path[50];
  strcpy(file_path, temp_node->message->voice_path);

  g_print("The message is being played: %s\n", file_path);

  // Lock the mutex to ensure no race conditions when accessing the pipeline
  pthread_mutex_lock(&play_mutex);

  // Create the GStreamer elements for the new pipeline
  GstElement *pipeline, *source, *decodebin, *sink;
  pipeline = gst_pipeline_new("audio-pipeline");
  source = gst_element_factory_make("filesrc", "source");
  decodebin = gst_element_factory_make("decodebin", "decodebin");
  sink = gst_element_factory_make("autoaudiosink", "sink");

  if (!pipeline || !source || !decodebin || !sink) {
    g_printerr("Failed to create GStreamer elements.\n");
    pthread_mutex_unlock(&play_mutex); // Unlock the mutex before returning
    return NULL;
  }

  g_print("GStreamer elements created successfully.\n");

  // Set the source file location
  g_object_set(G_OBJECT(source), "location", file_path, NULL);

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), source, decodebin, sink, NULL);
  g_print("Elements added to the pipeline.\n");

  // Link the source to decodebin
  gst_element_link(source, decodebin);
  g_print("Source linked to decodebin.\n");

  // Handle dynamic pads for decodebin
  g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), sink);
  g_print("Dynamic pad setup for decodebin.\n");

  // Set the current pipeline
  current_pipeline = pipeline;

  // Start playing the audio
  GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr("Failed to change pipeline state to PLAYING.\n");
    current_pipeline = NULL;           // Clear current pipeline on failure
    pthread_mutex_unlock(&play_mutex); // Unlock the mutex before returning
    return NULL;
  }

  g_print("Audio playback started.\n");

  // Non-blocking loop to monitor the bus
  GstBus *bus = gst_element_get_bus(pipeline);
  gboolean running = TRUE;
  while (running) {
    GstMessage *msg =
        gst_bus_timed_pop(bus, GST_MSECOND * 100); // Check every 100ms
    if (msg != NULL) {
      switch (GST_MESSAGE_TYPE(msg)) {
      case GST_MESSAGE_EOS:
        g_print("Audio playback completed (EOS).\n");
        running = FALSE;
        break;
      case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug_info;
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error: %s\n", err->message);
        g_free(debug_info);
        g_error_free(err);
        running = FALSE;
        break;
      }
      default:
        break;
      }
      gst_message_unref(msg);
    }
  }

  // Clean up the pipeline
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(bus);
  gst_object_unref(pipeline);
  current_pipeline = NULL;
  g_print("Pipeline cleaned up.\n");

  // Unlock the mutex to indicate the thread is done
  pthread_mutex_unlock(&play_mutex);
  g_print("Mutex unlocked after playback.\n");
  if (audio_thread != 0) {
    g_print("Cancelling and joining the previous audio thread.\n");
    pthread_cancel(audio_thread);     // Cancel the previous thread
    pthread_join(audio_thread, NULL); // Wait for the thread to finish
    audio_thread = 0;
    g_print("Previous audio thread stopped successfully.\n");
  }

  return NULL;
}
// Function to stop the current audio pipeline
void stop_audio() {
  g_print("Stopping audio playback...\n");
  pthread_mutex_unlock(&play_mutex);

  if (current_pipeline != NULL) {
    g_print("Setting pipeline state to NULL...\n");
    gst_element_set_state(current_pipeline, GST_STATE_NULL);
    gst_object_unref(current_pipeline);
    current_pipeline = NULL;
    if (audio_thread != 0) {
      g_print("Cancelling and joining the previous audio thread.\n");
      pthread_cancel(audio_thread);     // Cancel the previous thread
      pthread_join(audio_thread, NULL); // Wait for the thread to finish
      audio_thread = 0;
      g_print("Previous audio thread stopped successfully.\n");
    }
    g_print("Current audio pipeline stopped and cleared successfully.\n");
  } else {
    g_print("No active audio pipeline to stop.\n");
  }
}

// Function to stop the current audio thread (if any) and start a new one
void play_voice(GtkWidget *button, gpointer data) {
  g_print("play_voice function called.\n");

  // Check if a thread is already running, if so, stop playback
  if (audio_thread != 0) {
    g_print("Cancelling and joining the previous audio thread.\n");
    pthread_cancel(audio_thread);     // Cancel the previous thread
    pthread_join(audio_thread, NULL); // Wait for the thread to finish
    audio_thread = 0;
    g_print("Previous audio thread stopped successfully.\n");
  }

  // Stop any currently playing audio
  stop_audio();

  // Create a new thread to handle the audio playback
  if (pthread_create(&audio_thread, NULL, play_audio_thread, data) != 0) {
    g_printerr("Failed to create audio thread.\n");
    return;
  }

  g_print("New audio thread created successfully.\n");

  // Detach the thread so it can clean up itself when done
  pthread_detach(audio_thread);
  g_print("New audio thread detached.\n");
}

void show_message_menu(GtkWidget *message_button, gpointer data) {
  MessageNode *temp_node = (MessageNode *)data;
  if ((*temp_node).message->menu != NULL) {
    gtk_popover_popup(GTK_POPOVER((*temp_node).message->menu));
    gtk_widget_show_all((*temp_node).message->menu);
  }
}

void start_change_message(GtkWidget *change_message_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  GtkWidget *message_menu =
      gtk_widget_get_parent(gtk_widget_get_parent(change_message_button));
  MessageNode *changing_message;

  // Find the corresponding message node
  for (changing_message = (*main_page).messages;
       changing_message != NULL &&
       (*changing_message).message->menu != message_menu;
       changing_message = changing_message->next)
    ;

  if (changing_message == NULL) {
    g_print("Changing message not found.\n");
    return;
  }

  main_page->opened_chat->changing_message = changing_message;
  g_print("Changing message \"%s\"\n",
          main_page->opened_chat->changing_message->message->content);

  // Ensure valid UTF-8 and set the message content
  const char *message_content = changing_message->message->content;
  if (!message_content) {
    message_content = ""; // Default to an empty string
  } else if (!g_utf8_validate(message_content, -1, NULL)) {
    g_print("Error: Message content is not valid UTF-8.\n");
    message_content = ""; // Default to empty if invalid
  }

  gtk_text_buffer_set_text(main_page->message_buffer, message_content, -1);
}

void change_message(t_main_page_data *main_page, const gchar *message_text) {

  cJSON *json_message = cJSON_CreateObject();
  cJSON_AddStringToObject(json_message, "action", "UPDATE_MESSAGE");
  cJSON_AddNumberToObject(
      json_message, "message_id",
      main_page->opened_chat->changing_message->message->message_id);
  cJSON_AddNumberToObject(json_message, "chat_id", main_page->opened_chat->id);
  cJSON_AddStringToObject(json_message, "message", message_text);
  main_page->opened_chat->changing_message->message->status = MODIFIED;

  char *json_str = cJSON_Print(json_message);
  g_print("Sending message to server: %s\n", json_str);
  send(sock, json_str, strlen(json_str), 0);

  free(json_str);
  cJSON_free(json_message);
  // gtk_label_set_text(
  //     GTK_LABEL(
  //         main_page->opened_chat->changing_message->message->changed_label),
  //     "Modified");
  // g_print("message changed from \"%s\" to \"%s\"\n",
  //         main_page->opened_chat->changing_message->message->content,
  //         message_text);
  // main_page->opened_chat->changing_message = NULL;
}

void delete_message(GtkWidget *delete_message_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  GtkWidget *message_menu =
      gtk_widget_get_parent(gtk_widget_get_parent(delete_message_button));
  MessageNode *delete_message;
  for (delete_message = (*main_page).messages;
       delete_message != NULL &&
       (*delete_message).message->menu != message_menu;
       delete_message = delete_message->next)
    ;
  if (delete_message == NULL) {
    g_print("Deleting message does not found\n");
    return;
  }
  cJSON *json_message = cJSON_CreateObject();
  cJSON_AddStringToObject(json_message, "action", "DELETE_MESSAGE");
  cJSON_AddNumberToObject(json_message, "message_id",
                          delete_message->message->message_id);
  cJSON_AddNumberToObject(json_message, "chat_id", main_page->opened_chat->id);
  char *json_str = cJSON_Print(json_message);
  g_print("Sending message to server: %s\n", json_str);
  send(sock, json_str, strlen(json_str), 0);

  free(json_str);
  cJSON_free(json_message);
  (*delete_message).message->status = DELETED;
  strcpy(delete_message->message->content, "Deleted");
  if (main_page->opened_chat->changing_message == delete_message)
    main_page->opened_chat->changing_message = NULL;
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

gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event,
                            gpointer user_data) {
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
  g_free(json_str);
  g_free((gpointer)message);
}

MessageNode *create_message_node(t_main_page_data *main_page,
                                 ContentType message_type, int chat_id,
                                 cJSON *message_json) {
  // Create a new node
  MessageNode *temp_node = g_malloc(sizeof(MessageNode));

  if (!temp_node) {
    fprintf(stderr, "Failed to allocate memory for new message node.\n");
    return NULL;
  }
  temp_node->message = g_malloc(sizeof(MessageCache));

  if (!temp_node->message) {
    fprintf(stderr, "Failed to allocate memory for message.\n");
    g_free(temp_node); // Free the node if allocation fails
    return NULL;
  }

  // Initialize the message fields
  if ((*main_page).messages == NULL) {
    // Initialize the first node if the list is empty
    (*main_page).messages = temp_node;
  } else {
    // Otherwise, traverse to the end of the list
    MessageNode *last_node = (*main_page).messages;
    while (last_node->next != NULL) {
      last_node = last_node->next;
    }

    // Set the next pointer of the last node to the new node
    last_node->next = temp_node;
  }
  temp_node->next = NULL; // Make sure the new node's next is NULL
  // temp_node->next =
  //     (*main_page).messages;         // Insert at the beginning of the list
  // (*main_page).messages = temp_node; // Update the head of the list

  temp_node->message->chat_id = chat_id;
  temp_node->message->content_type = message_type;
  temp_node->message->status = NEW;

  // Set the values from the parsed JSON message
  cJSON *id_json = cJSON_GetObjectItem(message_json, "message_id");
  cJSON *content_json = cJSON_GetObjectItem(message_json, "content");
  cJSON *sender_json = cJSON_GetObjectItem(message_json, "username");
  cJSON *timestamp_json = cJSON_GetObjectItem(message_json, "timestamp");
  cJSON *read_json = cJSON_GetObjectItem(message_json, "read");
  cJSON *status_json = cJSON_GetObjectItem(message_json, "status");
  cJSON *voice_path_json = cJSON_GetObjectItem(
      message_json, "voice_file_path"); // Extract voice file path

  // Populate message fields

  if (cJSON_IsString(status_json)) {
    if (strcmp(status_json->valuestring, "new") == 0) {
      temp_node->message->status = NEW;
    } else if (strcmp(status_json->valuestring, "modified") == 0) {
      temp_node->message->status = MODIFIED;
    } else if (strcmp(status_json->valuestring, "deleted") == 0) {
      temp_node->message->status = DELETED;
    }
  }
  if (cJSON_IsString(content_json)) {
    strncpy(temp_node->message->content, content_json->valuestring,
            sizeof(temp_node->message->content) - 1);
  }

  if (cJSON_IsString(sender_json)) {
    strncpy(temp_node->message->sender, sender_json->valuestring,
            sizeof(temp_node->message->sender) - 1);
  }
  if (cJSON_IsNumber(id_json)) {
    temp_node->message->message_id = id_json->valueint;
    g_print("Message ID set to: %d\n",
            temp_node->message->message_id); // Debugging print
  } else {
    g_print(
        "Error: message_id not found or not a number.\n"); // Debugging error if
                                                           // missing or invalid
    temp_node->message->message_id = 0; // Set default ID if invalid
  }

  if (cJSON_IsString(timestamp_json)) {
    struct tm tm_utc = {0};

    // Parse the timestamp in the expected format "YYYY-MM-DD HH:MM:SS"
    if (sscanf(timestamp_json->valuestring, "%d-%d-%d %d:%d:%d",
               &tm_utc.tm_year, &tm_utc.tm_mon, &tm_utc.tm_mday,
               &tm_utc.tm_hour, &tm_utc.tm_min, &tm_utc.tm_sec) == 6) {

      tm_utc.tm_year -= 1900; // Adjust year to be relative to 1900
      tm_utc.tm_mon -= 1;     // Adjust month to be in range 0-11

      // Convert struct tm to time_t (in UTC)
      time_t utc_time = timegm(&tm_utc);
      if (utc_time != -1) {
        // Now convert to local time
        struct tm *tm_local = localtime(&utc_time);
        if (tm_local) {
          // Convert localtime back to time_t
          temp_node->message->date = mktime(tm_local); // Assign the local time
        } else {
          temp_node->message->date = 0; // Handle error if localtime fails
        }
      } else {
        temp_node->message->date = 0; // Handle error if mktime fails
      }
    }
  }

  if (cJSON_IsNumber(read_json)) {
    temp_node->message->read = read_json->valueint;
  }

  // Store voice path if the message is of type VOICE
  if (message_type == VOICE && cJSON_IsString(voice_path_json)) {
    strncpy(temp_node->message->voice_path, voice_path_json->valuestring,
            sizeof(temp_node->message->voice_path) - 1);
    g_print("filepath set to %s\n", temp_node->message->voice_path);
  }

  temp_node->message->button = NULL; // Initial value, you can modify later
  // If it's a voice message, create the button and connect the signal
  return temp_node;
}

// void test(gpointer *ptr) { g_print("test\n"); }

void create_message_button(t_main_page_data *main_page,
                           MessageNode *temp_node) {
  t_chat_node *chat_with_new_message;

  // Find the correct chat to add the message button
  for (chat_with_new_message = (*main_page).chats;
       chat_with_new_message->chat.id != (*temp_node).message->chat_id;
       chat_with_new_message = chat_with_new_message->next)
    ;
  if (temp_node->message->status == DELETED) {
    (*temp_node).message->button = gtk_button_new();
    gtk_box_pack_end(GTK_BOX(chat_with_new_message->chat.box),
                     (*temp_node).message->button, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(chat_with_new_message->chat.box),
                          (*temp_node).message->button, 0);
    if (strcmp(temp_node->message->sender, username) == 0) {
      // If the sender is the current user, it's an outgoing message
      gtk_widget_set_halign((*temp_node).message->button, GTK_ALIGN_END);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->button),
          "outgoing-messages");
    } else {
      // If the sender is not the current user, it's an incoming message
      gtk_widget_set_halign((*temp_node).message->button, GTK_ALIGN_START);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->button),
          "ingoing-messages");
    }
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER((*temp_node).message->button), main_box);
    (*temp_node).message->message_label = gtk_label_new("Deleted");
    gtk_box_pack_start(GTK_BOX(main_box), (*temp_node).message->message_label,
                       TRUE, FALSE, 0);
    gtk_label_set_xalign(GTK_LABEL((*temp_node).message->message_label), 0);
    gtk_label_set_justify(GTK_LABEL((*temp_node).message->message_label),
                          GTK_JUSTIFY_LEFT);
    gtk_label_set_line_wrap(GTK_LABEL((*temp_node).message->message_label),
                            TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL((*temp_node).message->message_label),
                                 PANGO_WRAP_WORD_CHAR);

    gtk_widget_set_visible((*temp_node).message->button, 1);
    gtk_widget_set_visible(main_box, 1);
    gtk_widget_set_visible((*temp_node).message->message_label, 1);
  } else {
    // Create a new button for the message
    (*temp_node).message->button = gtk_button_new();
    gtk_box_pack_end(GTK_BOX(chat_with_new_message->chat.box),
                     (*temp_node).message->button, FALSE, FALSE, 0);
    gtk_box_reorder_child(GTK_BOX(chat_with_new_message->chat.box),
                          (*temp_node).message->button, 0);

    // g_signal_connect(temp_node->message->button, "clicked", G_CALLBACK(test),
    //                  (gpointer)temp_node);

    // Compare with the message sender
    if (strcmp(temp_node->message->sender, username) == 0) {
      // If the sender is the current user, it's an outgoing message
      gtk_widget_set_halign((*temp_node).message->button, GTK_ALIGN_END);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->button),
          "outgoing-messages");
    } else {
      // If the sender is not the current user, it's an incoming message
      gtk_widget_set_halign((*temp_node).message->button, GTK_ALIGN_START);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->button),
          "ingoing-messages");
    }

    // Create the main box to hold the message content
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER((*temp_node).message->button), main_box);

    // Add a label with the sender's username
    if (strcmp(temp_node->message->sender, username) != 0) {
      (*temp_node).message->username_label =
          gtk_label_new((*temp_node).message->sender);
      gtk_box_pack_start(GTK_BOX(main_box),
                         (*temp_node).message->username_label, FALSE, FALSE, 0);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->username_label),
          "sender-name");
      gtk_widget_set_halign((*temp_node).message->username_label,
                            GTK_ALIGN_START);
    }

	GtkWidget *container;
    // If it's a text message, show the content
    if ((*temp_node).message->content_type == TEXT) {
      (*temp_node).message->message_label =
          gtk_label_new((*temp_node).message->content);
      gtk_box_pack_start(GTK_BOX(main_box), (*temp_node).message->message_label,
                         TRUE, FALSE, 0);
      gtk_label_set_xalign(GTK_LABEL((*temp_node).message->message_label), 0);
      gtk_label_set_justify(GTK_LABEL((*temp_node).message->message_label),
                            GTK_JUSTIFY_LEFT);
      gtk_label_set_line_wrap(GTK_LABEL((*temp_node).message->message_label),
                              TRUE);
      gtk_label_set_line_wrap_mode(
          GTK_LABEL((*temp_node).message->message_label), PANGO_WRAP_WORD_CHAR);
    } else if ((*temp_node).message->content_type == VOICE) {
      // If it's a voice message, show a button to play the voice message
      GtkWidget *play_button = gtk_image_new_from_file(
          "uchat-client/src/gui/resources/play-button.png");
      (*temp_node).message->voice_message_button = gtk_button_new();
      gtk_button_set_image(
          GTK_BUTTON((*temp_node).message->voice_message_button), play_button);
      gtk_box_pack_start(GTK_BOX(main_box),
                         (*temp_node).message->voice_message_button, FALSE,
                         FALSE, 0);
      gtk_style_context_add_class(
          gtk_widget_get_style_context(
              (*temp_node).message->voice_message_button),
          "play-button");
      gtk_widget_set_halign((*temp_node).message->voice_message_button,
                            GTK_ALIGN_START);
      // g_print("Connecting signal to play voice for message: %s\n",
      //         temp_node->message.voice_path);
      // Make sure the signal handler is connected to the voice button
      g_signal_connect(temp_node->message->voice_message_button, "clicked",
                       G_CALLBACK(play_voice), (gpointer)temp_node);
      (*temp_node).message->message_label = gtk_label_new("");
      gtk_box_pack_start(GTK_BOX(main_box), (*temp_node).message->message_label,
                         TRUE, FALSE, 0);
      gtk_label_set_xalign(GTK_LABEL((*temp_node).message->message_label), 0);
      gtk_label_set_justify(GTK_LABEL((*temp_node).message->message_label),
                            GTK_JUSTIFY_LEFT);
      gtk_label_set_line_wrap(GTK_LABEL((*temp_node).message->message_label),
                              TRUE);
      gtk_label_set_line_wrap_mode(
          GTK_LABEL((*temp_node).message->message_label), PANGO_WRAP_WORD_CHAR);
    } else if ((*temp_node).message->content_type == ANY_FILE) {
      // If it is file message, show save button and filename
      container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_box_pack_start(GTK_BOX(main_box), container, FALSE, FALSE, 5);
      (*temp_node).message->save_file_button = gtk_button_new();
      GtkWidget *save_file_button_image = gtk_image_new_from_file(
          "uchat-client/src/gui/resources/file-icon.png");
      gtk_button_set_image(
          GTK_BUTTON((*temp_node).message->save_file_button), save_file_button_image);
      gtk_box_pack_start(GTK_BOX(container),
                         (*temp_node).message->save_file_button,
                         FALSE, FALSE, 0);
      gtk_style_context_add_class(
          gtk_widget_get_style_context(
              (*temp_node).message->save_file_button), "save-button");
      //g_signal_connect(temp_node->message->save_file_button, "clicked",
      //                 G_CALLBACK(save_file), (gpointer)temp_node);
      (*temp_node).message->message_label = gtk_label_new((*temp_node).message->content);
      gtk_box_pack_start(GTK_BOX(container), (*temp_node).message->message_label,
                         TRUE, TRUE, 0);
      gtk_label_set_line_wrap(GTK_LABEL((*temp_node).message->message_label), TRUE);
      gtk_label_set_line_wrap_mode(
          GTK_LABEL((*temp_node).message->message_label), PANGO_WRAP_WORD_CHAR);
    } else if ((*temp_node).message->content_type == IMAGE) {
      // If it is image message, show image
      container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
      gtk_box_pack_start(GTK_BOX(main_box), container, FALSE, FALSE, 5);

      // Change to image filepath
      (*temp_node).message->image_file = resize_image_file((*temp_node).message->voice_path);
      gtk_box_pack_start(GTK_BOX(container),
                         (*temp_node).message->image_file, FALSE, FALSE, 0);
    }

    // Message menu
    if (strcmp(temp_node->message->sender, username) == 0) {
      (*temp_node).message->menu =
          gtk_popover_new((*temp_node).message->button);
      gtk_style_context_add_class(
          gtk_widget_get_style_context((*temp_node).message->menu),
          "smile-window");
      GtkWidget *message_menu_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
      gtk_container_add(GTK_CONTAINER((*temp_node).message->menu),
                        message_menu_box);
      g_signal_connect((*temp_node).message->button, "clicked",
                       G_CALLBACK(show_message_menu), temp_node);
      if ((*temp_node).message->content_type == TEXT) {
        GtkWidget *change_message_button = gtk_button_new_with_label("Change");
        gtk_style_context_add_class(
            gtk_widget_get_style_context(change_message_button),
            "popover-buttons");
        gtk_box_pack_start(GTK_BOX(message_menu_box), change_message_button,
                           TRUE, FALSE, 0);
        g_signal_connect(change_message_button, "clicked",
                         G_CALLBACK(start_change_message), main_page);
      }

      GtkWidget *delete_message_button = gtk_button_new_with_label("Delete");
      gtk_style_context_add_class(
          gtk_widget_get_style_context(delete_message_button),
          "popover-buttons");
      gtk_box_pack_start(GTK_BOX(message_menu_box), delete_message_button, TRUE,
                         FALSE, 0);
      g_signal_connect(delete_message_button, "clicked",
                       G_CALLBACK(delete_message), main_page);

    } else
      (*temp_node).message->menu = NULL;

    // Add the bottom box for labels (time, seen, changes)
    GtkWidget *bottom_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), bottom_box, TRUE, TRUE, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(bottom_box),
                                "chat-button-bottom-box");

    // Time label
    (*temp_node).message->time_label = gtk_label_new("00:00");
    char *time_str =
        ctime(&(*temp_node).message->date); // Convert time_t to string
    if (time_str) {
      time_str[strlen(time_str) - 1] = '\0'; // Remove the newline character
      gtk_label_set_text(GTK_LABEL((*temp_node).message->time_label), time_str);
    }
    gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message->time_label,
                     FALSE, FALSE, 0);
    gtk_style_context_add_class(
        gtk_widget_get_style_context((*temp_node).message->time_label),
        "small-inscriptions");
    gtk_widget_set_halign((*temp_node).message->time_label, GTK_ALIGN_END);

    // Changed label
    if (temp_node->message->status == MODIFIED)
      (*temp_node).message->changed_label = gtk_label_new("Edited");
    else
      (*temp_node).message->changed_label = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message->changed_label,
                     FALSE, FALSE, 0);
    gtk_style_context_add_class(
        gtk_widget_get_style_context((*temp_node).message->changed_label),
        "small-inscriptions");
    gtk_widget_set_halign((*temp_node).message->changed_label, GTK_ALIGN_END);

    // Seen label
    (*temp_node).message->seen_label = gtk_label_new("");
    gtk_box_pack_end(GTK_BOX(bottom_box), (*temp_node).message->seen_label,
                     FALSE, FALSE, 0);
    gtk_style_context_add_class(
        gtk_widget_get_style_context((*temp_node).message->seen_label),
        "small-inscriptions");
    gtk_widget_set_halign((*temp_node).message->seen_label, GTK_ALIGN_END);

    // Set visibility for all elements
    gtk_widget_set_visible((*temp_node).message->button, 1);
    gtk_widget_set_visible(main_box, 1);
    if (strcmp(temp_node->message->sender, username) != 0)
      gtk_widget_set_visible((*temp_node).message->username_label, 1);
    gtk_widget_set_visible(bottom_box, 1);
    if ((*temp_node).message->content_type == TEXT)
      gtk_widget_set_visible((*temp_node).message->message_label, 1);
    else if ((*temp_node).message->content_type == VOICE)
      gtk_widget_set_visible((*temp_node).message->voice_message_button, 1);
    else if ((*temp_node).message->content_type == ANY_FILE) {
      gtk_widget_set_visible(container, 1);
      gtk_widget_set_visible((*temp_node).message->message_label, 1);
      gtk_widget_set_visible((*temp_node).message->save_file_button, 1);
    }
    else if ((*temp_node).message->content_type == IMAGE) {
      gtk_widget_set_visible(container, 1);
      gtk_widget_set_visible((*temp_node).message->image_file, 1);
    }
    gtk_widget_set_visible((*temp_node).message->changed_label, 1);
    gtk_widget_set_visible((*temp_node).message->time_label, 1);
    gtk_widget_set_visible((*temp_node).message->seen_label, 1);
  }
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
    if (main_page->opened_chat->changing_message != NULL)
      change_message(main_page, message_text);
    else
      send_message_to_server(chat_id, message_text);
    // MessageNode *new_message = create_message_node(main_page, TEXT, chat_id);
    // create_message_button(main_page, new_message);
  } else {
    g_print("Cannot send an empty message.\n");
  }
  gtk_text_buffer_set_text(message_buffer, "", -1);
}
