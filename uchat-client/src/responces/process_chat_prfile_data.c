#include <uchat.h>

GtkWidget *profile_window = NULL;

void process_chat_profile_data(const char *json_response, AppData *app_data) {

  //apply_css(profile_window, "uchat-client/src/gui/resources/styles.css");
  cJSON *response = cJSON_Parse(json_response);
  if (!response) {
    fprintf(stderr, "Failed to parse JSON response.\n");
    return;
  }

  cJSON *type = cJSON_GetObjectItem(response, "type");
  cJSON *members = cJSON_GetObjectItem(response, "members");

  if (!cJSON_IsString(type) || !cJSON_IsArray(members)) {
    fprintf(stderr, "Invalid JSON format for chat profile data.\n");
    cJSON_Delete(response);
    return;
  }

  // If the profile window already exists, destroy it to redraw
  if (GTK_IS_WIDGET(profile_window)) {
    gtk_widget_destroy(profile_window);
    profile_window = NULL;
  }

  // Create a new profile window
  profile_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gboolean is_group = (strcmp(type->valuestring, "private_group") == 0);

  gtk_window_set_title(GTK_WINDOW(profile_window),
                       is_group ? "Group Profile" : "User Profile");
  gtk_window_set_default_size(GTK_WINDOW(profile_window), 400, 500);
  gtk_window_set_position(GTK_WINDOW(profile_window), GTK_WIN_POS_CENTER);

  // Main container for the profile window
  GtkWidget *main_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(profile_window), main_container);
  gtk_style_context_add_class(gtk_widget_get_style_context(main_container),
                              "main-page-form");
  
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(main_container), content_box, TRUE, FALSE, 0);
  gtk_widget_set_size_request(GTK_WIDGET(content_box), 375, -1);
  gtk_widget_set_halign(content_box, GTK_ALIGN_CENTER);
  gtk_style_context_add_class(gtk_widget_get_style_context(content_box),
  							  "form");

  GtkWidget *title_label =
      gtk_label_new(is_group ? "Group Members" : "User Details");
  gtk_box_pack_start(GTK_BOX(content_box), title_label, FALSE, FALSE, 0);
  gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
  gtk_style_context_add_class(gtk_widget_get_style_context(title_label),
                              "form-name-label");

  // Profile picture
  const char *avatar_path =
      is_group ? "uchat-client/src/gui/resources/rabbits_group.png"
               : "uchat-client/src/gui/resources/rabbit_big.png";
  GdkPixbuf *original_avatar = gdk_pixbuf_new_from_file(
      avatar_path, NULL);
  GdkPixbuf *resized_avatar =
      gdk_pixbuf_scale_simple(original_avatar, 150, 150, GDK_INTERP_BILINEAR);
  GtkWidget *avatar = gtk_image_new_from_pixbuf(resized_avatar);
  g_object_unref(original_avatar);
  g_object_unref(resized_avatar);
  gtk_box_pack_start(GTK_BOX(content_box), avatar, FALSE, FALSE, 0);
  gtk_widget_set_halign(avatar, GTK_ALIGN_CENTER);
  gtk_style_context_add_class(gtk_widget_get_style_context(avatar),
                              "img-edit");

  // Members info
  GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start(GTK_BOX(content_box), info_box, TRUE, TRUE, 0);
  int i = 0;

  cJSON *member;
  cJSON_ArrayForEach(member, members) {
    const char *Username = cJSON_GetObjectItem(member, "username")->valuestring;
    const char *status = cJSON_GetObjectItem(member, "status")->valuestring;
    const char *full_name =
        cJSON_GetObjectItem(member, "full_name")->valuestring;
    const char *group = cJSON_GetObjectItem(member, "group")->valuestring;
    const char *role = cJSON_GetObjectItem(member, "role")->valuestring;

    // Create a member info box
    GtkWidget *member_grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(info_box), member_grid, FALSE, FALSE, 0);
    gtk_grid_set_row_homogeneous(GTK_GRID(member_grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(member_grid), TRUE);

    // Username and status for all members in a group
    if (strcmp(username, Username) != 0 && !is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      gtk_box_pack_start(GTK_BOX(info_box), username_label, TRUE, TRUE, 0);
      gtk_style_context_add_class(
      	gtk_widget_get_style_context(username_label), "profile-name");

      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "online" : "offline");
      const char *status_class = strcmp(status, "online") == 0 ? "status" : "form-message";
      gtk_box_pack_start(GTK_BOX(info_box), status_label, TRUE, TRUE, 0);
      gtk_style_context_add_class(
      	gtk_widget_get_style_context(status_label), status_class);
    } else if (is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "online" : "offline");
      const char *status_class = strcmp(status, "online") == 0 ? "form-message-success" : "form-message";

      GtkStyleContext *status_context = gtk_widget_get_style_context(status_label);
      gtk_style_context_add_class(status_context, status_class);
      gtk_grid_attach(GTK_GRID(member_grid), username_label, 0, i, 1, 1);
      gtk_widget_set_halign(username_label, GTK_ALIGN_CENTER);
      gtk_grid_attach(GTK_GRID(member_grid), status_label, 1, i, 1, 1);
      gtk_widget_set_hexpand(username_label, TRUE);
      gtk_widget_set_hexpand(status_label, TRUE);
      i++;
    }

    if (!is_group && strcmp(username, Username) != 0) {
      // Additional details for private chats
      GtkWidget *name_value = gtk_label_new(full_name);
  	  gtk_style_context_add_class(
      gtk_widget_get_style_context(name_value), "form-label");

      GtkWidget *group_value = gtk_label_new(group);
        	gtk_style_context_add_class(
      gtk_widget_get_style_context(group_value), "form-label");

      GtkWidget *role_value = gtk_label_new(role);
        	gtk_style_context_add_class(
      gtk_widget_get_style_context(role_value), "form-label");

      gtk_box_pack_start(GTK_BOX(info_box), name_value, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(info_box), group_value, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(info_box), role_value, FALSE, FALSE, 0);

      gtk_widget_set_halign(name_value, GTK_ALIGN_CENTER);
      gtk_widget_set_halign(group_value, GTK_ALIGN_CENTER);
      gtk_widget_set_halign(role_value, GTK_ALIGN_CENTER);
    }
  }

  gtk_widget_show_all(profile_window);
  cJSON_Delete(response);
}
