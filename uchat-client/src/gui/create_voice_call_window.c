#include "uchat.h"

int incoming = 0;

void close_voice_call_window(GtkWidget *voice_call_window,
                             t_main_page_data *main_page) {
  gtk_widget_destroy(voice_call_window);
  main_page->voice_call_window = NULL;
}

void stop_voice_call(GtkWidget *reject_button, t_main_page_data *main_page) {

  cJSON *response = cJSON_CreateObject();
  cJSON_AddStringToObject(response, "action", "STOP_CALL");

  char *response_str = cJSON_Print(response);
  cJSON_free(response);

  if (send(sock, response_str, strlen(response_str), 0) == -1) {
    perror("Failed to send status to sender");
  }

  free(response_str);
  const gchar *label =
      gtk_label_get_text(GTK_LABEL(main_page->voice_call_window_label));
  if (strcmp(label, "Offline") == 0) {
    close_voice_call_window(main_page->voice_call_window, main_page);
  }
}

void accept_voice_call(GtkWidget *accept_button, t_main_page_data *main_page) {
  start_receive_pipeline();
  if (send(sock, main_page->accept_call, strlen(main_page->accept_call), 0) ==
      -1) {
    stop_receive_pipeline();
    perror("Failed to send status to sender");
    return;
  }
  cJSON *temp = cJSON_Parse(main_page->accept_call);
  cJSON *caller_ip = cJSON_GetObjectItem(temp, "caller_ipp");
  cJSON *caller_port = cJSON_GetObjectItem(temp, "caller_port");
  cJSON *caller_name = cJSON_GetObjectItem(temp, "caller_name");
  start_send_pipeline(caller_ip->valuestring, caller_port->valueint);
  in_call = 1;
  incoming = 0;
  cJSON_free(temp);
  close_voice_call_window(main_page->voice_call_window, main_page);
  create_voice_call_window(NULL, main_page, caller_name->valuestring);
  gtk_label_set_text(GTK_LABEL(main_page->voice_call_window_label), "In call");
}

void create_voice_call_window(GtkWidget *voice_call_button,
                              t_main_page_data *main_page, char *caller) {
  if (main_page->voice_call_window) {
    g_print("window exists\n");
    return;
  }

  main_page->voice_call_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_page->voice_call_window), "Voice Call");
  gtk_window_set_default_size(GTK_WINDOW(main_page->voice_call_window), 960,
                              540);
  gtk_window_set_position(GTK_WINDOW(main_page->voice_call_window),
                          GTK_WIN_POS_CENTER);
  g_signal_connect(main_page->voice_call_window, "destroy",
                   G_CALLBACK(close_voice_call_window), main_page);

  GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(main_page->voice_call_window),
                    main_container);
  gtk_style_context_add_class(gtk_widget_get_style_context(main_container),
                              "voice-call");

  GtkWidget *profile_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(main_container), profile_container, TRUE, TRUE, 0);
  gtk_widget_set_size_request(profile_container, 250, -1);
  gtk_widget_set_valign(profile_container, GTK_ALIGN_CENTER);

  GdkPixbuf *original_ava = gdk_pixbuf_new_from_file(
      "uchat-client/src/gui/resources/rabbit_profile.png", NULL);
  GdkPixbuf *resized_ava =
      gdk_pixbuf_scale_simple(original_ava, 200, 200, GDK_INTERP_BILINEAR);
  GtkWidget *avatar = gtk_image_new_from_pixbuf(resized_ava);
  gtk_box_pack_start(GTK_BOX(profile_container), avatar, FALSE, FALSE, 0);

  GtkWidget *username_label =
      gtk_label_new(gtk_label_get_text(GTK_LABEL(main_page->chat_nickname)));
  gtk_box_pack_start(GTK_BOX(profile_container), username_label, FALSE, FALSE,
                     0);
  gtk_style_context_add_class(gtk_widget_get_style_context(username_label),
                              "voice-call-username");

  main_page->voice_call_window_label = gtk_label_new("calling...");
  gtk_box_pack_start(GTK_BOX(profile_container),
                     main_page->voice_call_window_label, FALSE, FALSE, 0);
  gtk_style_context_add_class(
      gtk_widget_get_style_context(main_page->voice_call_window_label),
      "voice-call-status");

  GtkWidget *buttons_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(main_container), buttons_container, FALSE, FALSE,
                     0);
  gtk_widget_set_halign(buttons_container, GTK_ALIGN_CENTER);

  if (strcmp(caller, "") == 0) {
    GtkWidget *reject_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(buttons_container), reject_button, FALSE, FALSE,
                       25);
    gtk_style_context_add_class(gtk_widget_get_style_context(reject_button),
                                "voice-call-button");
    GtkWidget *reject_image = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-call-reject.png");
    gtk_button_set_image(GTK_BUTTON(reject_button), reject_image);
    g_signal_connect(reject_button, "clicked", G_CALLBACK(stop_voice_call),
                     main_page);
  } else if (strcmp(caller, "") != 0 && incoming == 0) {
    GtkWidget *reject_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(buttons_container), reject_button, FALSE, FALSE,
                       25);
    gtk_style_context_add_class(gtk_widget_get_style_context(reject_button),
                                "voice-call-button");
    GtkWidget *reject_image = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-call-reject.png");
    gtk_button_set_image(GTK_BUTTON(reject_button), reject_image);
    g_signal_connect(reject_button, "clicked", G_CALLBACK(stop_voice_call),
                     main_page);
    gtk_label_set_text(GTK_LABEL(username_label), caller);
  } else if (strcmp(caller, "") != 0 && incoming == 1) {
    GtkWidget *accept_button = gtk_button_new();
    gtk_box_pack_start(GTK_BOX(buttons_container), accept_button, FALSE, FALSE,
                       25);
    gtk_style_context_add_class(gtk_widget_get_style_context(accept_button),
                                "voice-call-button");
    GtkWidget *accept_image = gtk_image_new_from_file(
        "uchat-client/src/gui/resources/voice-call-accept.png");
    gtk_button_set_image(GTK_BUTTON(accept_button), accept_image);
    g_signal_connect(accept_button, "clicked", G_CALLBACK(accept_voice_call),
                     main_page);
    gtk_label_set_text(GTK_LABEL(username_label), caller);
  }

  gtk_widget_show_all(main_page->voice_call_window);
  if (in_call == 0 && incoming == 0) {
    const gchar *callee_name = gtk_label_get_text(GTK_LABEL(username_label));

    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "action", "CALL");
    cJSON_AddNumberToObject(response, "caller_port", receive_port);
    cJSON_AddStringToObject(response, "callee_name", callee_name);

    char *response_str = cJSON_Print(response);
    cJSON_free(response);

    if (send(sock, response_str, strlen(response_str), 0) == -1) {
      perror("Failed to send status to sender");
    }
    is_calling = 1;

    free(response_str);
  }
}
