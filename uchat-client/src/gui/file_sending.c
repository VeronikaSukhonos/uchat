#include <uchat.h>

// remove
MessageNode *tmp_create_message_node(t_main_page_data *main_page, ContentType message_type, int chat_id, char *filename, char *filepath) {
  MessageNode *temp_node = g_malloc(sizeof(MessageNode));
  if (!temp_node) {
    fprintf(stderr, "Failed to allocate memory for new message node.\n");
    return NULL;
  }

  temp_node->message = g_malloc(sizeof(MessageCache));
  if (!temp_node->message) {
    fprintf(stderr, "Failed to allocate memory for message.\n");
    g_free(temp_node);
    return NULL;
  }

  if ((*main_page).messages == NULL) {
    (*main_page).messages = temp_node;
  } else {
    MessageNode *last_node = (*main_page).messages;
    while (last_node->next != NULL) {
      last_node = last_node->next;
    }

    last_node->next = temp_node;
  }
  temp_node->next = NULL;

  temp_node->message->chat_id = chat_id;
  temp_node->message->content_type = message_type;
  temp_node->message->status = NEW;

  strncpy(temp_node->message->content, filename,
            sizeof(temp_node->message->content) - 1);
  strncpy(temp_node->message->sender, "sender",
            sizeof(temp_node->message->sender) - 1);
  temp_node->message->message_id = 0;
  temp_node->message->date = 0;

  strncpy(temp_node->message->voice_path, filepath,
          sizeof(temp_node->message->voice_path) - 1);
  g_print("filepath set to %s\n", temp_node->message->voice_path);

  temp_node->message->button = NULL;
  return temp_node;
}

void send_file_message(int sock, char *filepath, int chat_id, t_main_page_data *main_page) {
	char *filename = g_path_get_basename(filepath);
	GFile *file = g_file_new_for_path(filepath); 
	GFileInfo *info = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);

	if (info == NULL)
		g_print("Unable to get file info\n");
	else {
		if (info != NULL) {
			guint64 size = g_file_info_get_size(info);

			if (size < 15 * 1024 * 1024) {
				const char *type = g_file_info_get_content_type(info);

				if (g_str_has_prefix(type, "image/")) {
					g_print("File is image\n");
					//remove
					MessageNode *new_message = tmp_create_message_node(main_page, IMAGE, chat_id, filename, filepath);
					create_message_button(main_page, new_message);
				}
				else {
					g_print("File is NOT image\n");
					//remove
					MessageNode *new_message = tmp_create_message_node(main_page, ANY_FILE, chat_id, filename, filepath);
					create_message_button(main_page, new_message);
				}
			}
		}
		g_object_unref(info);
	}
	g_object_unref(file);
	g_free(filepath);
	g_free(filename);
}

GtkWidget *resize_image_file(char *filepath) {
	GdkPixbuf *original_image = gdk_pixbuf_new_from_file(filepath, NULL);
	int original_width = gdk_pixbuf_get_width(original_image);
	int original_height = gdk_pixbuf_get_height(original_image);

	GdkPixbuf *resized_image
		= gdk_pixbuf_scale_simple(original_image, 400,
								  (original_height * 400) / original_width,
								  GDK_INTERP_BILINEAR);

	GtkWidget *image = gtk_image_new_from_pixbuf(resized_image);

	g_object_unref(original_image);
	g_object_unref(resized_image);

	return image;
}

void show_filechooser(GtkWidget *attach_button, t_main_page_data *main_page) {
	GtkWidget *main_window = gtk_widget_get_parent(gtk_widget_get_parent(
		gtk_widget_get_parent(gtk_widget_get_parent(main_page->central_area_stack))));
	GtkWidget *choose_file_dialog =
		gtk_file_chooser_dialog_new("Choose a file",
									GTK_WINDOW(main_window),
									GTK_FILE_CHOOSER_ACTION_OPEN,
									"Open", GTK_RESPONSE_ACCEPT,
									"Cancel", GTK_RESPONSE_CANCEL, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(choose_file_dialog),
										 TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(choose_file_dialog),
										g_get_home_dir());
	GtkWidget *attach_image =
		gtk_image_new_from_file("uchat-client/src/gui/resources/attach-file-active.png");
		gtk_button_set_image(GTK_BUTTON(attach_button), attach_image);

	gint res = gtk_dialog_run(GTK_DIALOG(choose_file_dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		GSList *filelist = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(choose_file_dialog));
		GSList *ptr = filelist;
		while (ptr != NULL) {
			// send each file
			char *filepath = ptr->data;
			if (filepath != NULL) {
				g_print("Chosen file: %s\n", filepath);
				send_file_message(main_page->sock, filepath, main_page->opened_chat->id, main_page);
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

void on_drag_data_received(GtkWidget *dialog_scroll, GdkDragContext *c, gint x, gint y,
                           GtkSelectionData *data, guint info, guint time,
                           t_main_page_data *main_page) {
	gchar **uris = gtk_selection_data_get_uris(data);

	if (uris != NULL) {
		for (int i = 0; uris[i] != NULL; i++) {
        	gchar *filepath = g_filename_from_uri(uris[i], NULL, NULL);
        	if (filepath != NULL) {
        		// send each file
        		g_print("Chosen file: %s\n", filepath);
        		send_file_message(main_page->sock, filepath, main_page->opened_chat->id, main_page);
        	}
    	}
	}
	g_strfreev(uris);
}

