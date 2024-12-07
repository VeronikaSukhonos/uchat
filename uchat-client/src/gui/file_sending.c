#include <uchat.h>
const char *get_file_extension(const char *file_path) {
  const char *dot = strrchr(file_path, '.');
  if (!dot || dot == file_path) {
    return NULL; // No extension found
  }
  return dot + 1; // Return the extension (without the dot)
}
int is_image(char *file_path) {
  const char *type = get_file_extension(file_path);
  if (strcmp(type, "png") == 0) {
    return 1;
  } else if (strcmp(type, "jpg") == 0) {
    return 1;
  } else if (strcmp(type, "jpeg") == 0) {
    return 1;
  } else if (strcmp(type, "bmp") == 0) {
    return 1;
  } else {
    return 0;
  }
}

void send_file_message(int sock, char *file_path, int chat_id,
                       t_main_page_data *main_page) {
  char *filename = g_path_get_basename(file_path);
  GFile *file = g_file_new_for_path(file_path);
  GFileInfo *info = g_file_query_info(file, "standard::*",
                                      G_FILE_QUERY_INFO_NONE, NULL, NULL);
  char *encoded_file = read_and_encode_file(file_path);
  if (!encoded_file) {
    g_printerr("Failed to encode file: %s\n", file_path);
    return;
  }
  const char *type;
  if (info != NULL) {
    guint64 size = g_file_info_get_size(info);

    if (size < 15 * 1024 * 1024) {
      type = get_file_extension(file_path);
    }
  }
  // Create JSON object for the message
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "SEND_FILE_MESSAGE_TO_CHAT");
  cJSON_AddNumberToObject(json, "chat_id", chat_id);
  cJSON_AddStringToObject(json, "file_path", filename);
  cJSON_AddStringToObject(json, "file_type", type);
  cJSON_AddStringToObject(json, "file",
                          encoded_file); // Add encoded file content

  // Convert JSON object to string
  char *json_string = cJSON_PrintUnformatted(json);

  // Ensure the entire JSON message is sent
  ssize_t sent = send(sock, json_string, strlen(json_string), 0);
  if (sent < 0) {
    perror("Failed to send voice message");
  } else {
    g_print("File message sent successfully. with size %zi\n", sent);
  }
  usleep(100000); // Add a 100ms delay between sends

  // Cleanup
  g_free(encoded_file);
  g_free(json_string);
  cJSON_Delete(json);

  g_object_unref(file);
  g_free(file_path);
  g_free(filename);
}

GtkWidget *resize_image_file(char *filepath) {
  GdkPixbuf *original_image = gdk_pixbuf_new_from_file(filepath, NULL);
  int original_width = gdk_pixbuf_get_width(original_image);
  int original_height = gdk_pixbuf_get_height(original_image);

  GdkPixbuf *resized_image = gdk_pixbuf_scale_simple(
      original_image, 400, (original_height * 400) / original_width,
      GDK_INTERP_BILINEAR);

  GtkWidget *image = gtk_image_new_from_pixbuf(resized_image);

  g_object_unref(original_image);
  g_object_unref(resized_image);

  return image;
}

void show_filechooser(GtkWidget *attach_button, t_main_page_data *main_page) {
  GtkWidget *main_window =
      gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(
          gtk_widget_get_parent(main_page->central_area_stack))));
  GtkWidget *choose_file_dialog = gtk_file_chooser_dialog_new(
      "Choose a file", GTK_WINDOW(main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
      "Open", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(choose_file_dialog),
                                       TRUE);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(choose_file_dialog),
                                      g_get_home_dir());
  GtkWidget *attach_image = gtk_image_new_from_file(
      "uchat-client/src/gui/resources/attach-file-active.png");
  gtk_button_set_image(GTK_BUTTON(attach_button), attach_image);

  gint res = gtk_dialog_run(GTK_DIALOG(choose_file_dialog));
  if (res == GTK_RESPONSE_ACCEPT) {
    GSList *filelist =
        gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(choose_file_dialog));
    GSList *ptr = filelist;
    while (ptr != NULL) {
      // send each file
      char *filepath = ptr->data;
      if (filepath != NULL) {
        g_print("Chosen file: %s\n", filepath);
        send_file_message(main_page->sock, filepath, main_page->opened_chat->id,
                          main_page);
      }
      ptr = ptr->next;
    }
    g_slist_free(filelist);
  }
  attach_image =
      gtk_image_new_from_file("uchat-client/src/gui/resources/attach-file.png");
  gtk_button_set_image(GTK_BUTTON(attach_button), attach_image);
  gtk_widget_destroy(choose_file_dialog);
}

void on_drag_data_received(GtkWidget *dialog_scroll, GdkDragContext *c, gint x,
                           gint y, GtkSelectionData *data, guint info,
                           guint time, t_main_page_data *main_page) {
  gchar **uris = gtk_selection_data_get_uris(data);

  if (uris != NULL) {
    for (int i = 0; uris[i] != NULL; i++) {
      gchar *filepath = g_filename_from_uri(uris[i], NULL, NULL);
      if (filepath != NULL) {
        // send each file
        g_print("Chosen file: %s\n", filepath);
        send_file_message(main_page->sock, filepath, main_page->opened_chat->id,
                          main_page);
      }
    }
  }
  g_strfreev(uris);
}
