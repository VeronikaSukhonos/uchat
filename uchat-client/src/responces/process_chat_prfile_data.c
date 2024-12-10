#include <uchat.h>

void process_chat_profile_data(const char *json_response, AppData *app_data) {
  static GtkWidget *profile_window = NULL;

  apply_css(profile_window, "uchat-client/src/gui/resources/styles.css");
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
  if (profile_window) {
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
  gtk_container_set_border_width(GTK_CONTAINER(profile_window), 20);

  // Main container for the profile window
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_container_add(GTK_CONTAINER(profile_window), content_box);

  // Profile picture
  const char *avatar_path =
      is_group ? "uchat-client/src/gui/resources/rabbits_group.png"
               : "uchat-client/src/gui/resources/rabbit_profile.png";
  GtkWidget *avatar = gtk_image_new_from_file(avatar_path);
  gtk_widget_set_name(avatar, "avatar");
  gtk_box_pack_start(GTK_BOX(content_box), avatar, FALSE, FALSE, 0);
  gtk_widget_set_size_request(avatar, 150, 150);
  gtk_widget_set_halign(avatar, GTK_ALIGN_CENTER);

  // Title: Username/Group name
  GtkWidget *title_label =
      gtk_label_new(is_group ? "Group Members" : "User Details");
  gtk_widget_set_name(title_label, "title");
  gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(content_box), title_label, FALSE, FALSE, 0);

  // Members info
  GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_name(info_box, "info-box");
  gtk_box_pack_start(GTK_BOX(content_box), info_box, TRUE, TRUE, 10);
  gtk_widget_set_name(info_box, "fade-in");

  cJSON *member;
  cJSON_ArrayForEach(member, members) {
    const char *Username = cJSON_GetObjectItem(member, "username")->valuestring;
    const char *status = cJSON_GetObjectItem(member, "status")->valuestring;
    const char *full_name =
        cJSON_GetObjectItem(member, "full_name")->valuestring;
    const char *group = cJSON_GetObjectItem(member, "group")->valuestring;
    const char *role = cJSON_GetObjectItem(member, "role")->valuestring;

    // Create a member info box
    GtkWidget *member_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(member_box, "member-box");
    gtk_box_pack_start(GTK_BOX(info_box), member_box, FALSE, FALSE, 5);

    // Username and status for all members in a group
    if (strcmp(username, Username) != 0 && !is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "Online" : "Offline");
      const char *status_class = strcmp(status, "online") == 0 ? "status-online" : "status-offline";

      GtkStyleContext *status_context = gtk_widget_get_style_context(status_label);      
      gtk_style_context_add_class(status_context, status_class);
      gtk_box_pack_start(GTK_BOX(member_box), username_label, TRUE, TRUE, 5);
      gtk_box_pack_start(GTK_BOX(member_box), status_label, TRUE, TRUE, 5);
     


    } else if (is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "Online" : "Offline");
      const char *status_class = strcmp(status, "online") == 0 ? "status-online" : "status-offline";

      GtkStyleContext *status_context = gtk_widget_get_style_context(status_label);
      gtk_style_context_add_class(status_context, status_class);
      gtk_box_pack_start(GTK_BOX(member_box), username_label, TRUE, TRUE, 5);
      gtk_box_pack_start(GTK_BOX(member_box), status_label, TRUE, TRUE, 5);
    }

    if (!is_group && strcmp(username, Username) != 0) {
      // Additional details for private chats
      GtkWidget *details_grid = gtk_grid_new();
      gtk_grid_set_row_spacing(GTK_GRID(details_grid), 0);
      gtk_grid_set_column_spacing(GTK_GRID(details_grid), 0);
      gtk_box_pack_start(GTK_BOX(info_box), details_grid, FALSE, FALSE, 0);

      GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

      
      GtkStyleContext *context = gtk_widget_get_style_context(separator);
      gtk_style_context_add_class(context, "white-line");

      
      GtkWidget *name_label = gtk_label_new("Name:");
      GtkWidget *name_value = gtk_label_new(full_name);
      gtk_widget_set_name(name_value, "name");

      GtkWidget *group_label = gtk_label_new("Group:");
      GtkWidget *group_value = gtk_label_new(group);
      gtk_widget_set_name(group_value, "group");

      GtkWidget *role_label = gtk_label_new("Role:");
      GtkWidget *role_value = gtk_label_new(role);
      gtk_widget_set_name(role_value, "role");

      
      GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
      GtkWidget *group_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
      GtkWidget *role_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

      
      gtk_box_pack_start(GTK_BOX(name_box), name_label, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(name_box), name_value, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(group_box), group_label, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(group_box), group_value, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(role_box), role_label, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(role_box), role_value, FALSE, FALSE, 0);

      
      gtk_widget_set_halign(name_box, GTK_ALIGN_CENTER);
      gtk_widget_set_halign(group_box, GTK_ALIGN_CENTER);
      gtk_widget_set_halign(role_box, GTK_ALIGN_CENTER);
    
      gtk_box_set_spacing(GTK_BOX(content_box), 0);
      
      gtk_box_pack_start(GTK_BOX(content_box), separator, FALSE, FALSE, 0);
      gtk_box_pack_start(GTK_BOX(content_box), name_box, FALSE, FALSE, 5);
      gtk_box_pack_start(GTK_BOX(content_box), group_box, FALSE, FALSE, 10);
      gtk_box_pack_start(GTK_BOX(content_box), role_box, FALSE, FALSE, 10);


      gtk_grid_attach(GTK_GRID(details_grid), name_label, 0, 0, 1, 1);
      gtk_grid_attach(GTK_GRID(details_grid), name_value, 1, 0, 1, 1);

      gtk_grid_attach(GTK_GRID(details_grid), group_label, 0, 1, 1, 1);
      gtk_grid_attach(GTK_GRID(details_grid), group_value, 1, 1, 1, 1);

      gtk_grid_attach(GTK_GRID(details_grid), role_label, 0, 2, 1, 1);
      gtk_grid_attach(GTK_GRID(details_grid), role_value, 1, 2, 1, 1);
    }
  }

  gtk_widget_show_all(profile_window);
  cJSON_Delete(response);
}