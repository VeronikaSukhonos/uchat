#include <uchat.h>

void handle_logout(AppData *app_data) {
  if (app_data->main_page->voice_call_window)
    close_voice_call_window(app_data->main_page->voice_call_window,
                            app_data->main_page);

  if (app_data->main_page->opened_chat != NULL &&
      app_data->main_page->opened_chat->is_mic_active == TRUE) {
    stop_recording();
    GtkWidget *mic_button_img_start = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-start.png");
    gtk_button_set_image(GTK_BUTTON(app_data->main_page->mic_button),
                         mic_button_img_start);
    app_data->main_page->opened_chat->is_mic_active = FALSE;
  }
  app_data->main_page->opened_chat = NULL;
  delete_cache_directory();
  remove_buttons(app_data->main_page);
  remove_all_chat_buttons(app_data->main_page);
  free_message_list(app_data->main_page->messages);
  app_data->main_page->messages = NULL;
  g_print("Message head set to null\n");
  gtk_text_buffer_set_text(app_data->main_page->message_buffer, "", -1);
  gtk_stack_set_visible_child_name(GTK_STACK(app_data->pages), "login");
  gtk_window_set_focus(GTK_WINDOW(gtk_widget_get_parent(app_data->main_overlay)), NULL);
  delete_session();
}
