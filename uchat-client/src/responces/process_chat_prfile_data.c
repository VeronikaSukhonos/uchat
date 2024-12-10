#include <uchat.h>

void process_chat_profile_data(const char *json_response, AppData *app_data) {
  static GtkWidget *profile_window = NULL;

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
  GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
  gtk_container_add(GTK_CONTAINER(profile_window), content_box);

  // Profile picture
  const char *avatar_path =
      is_group ? "uchat-client/src/gui/resources/rabbits_group.png"
               : "uchat-client/src/gui/resources/rabbit_profile.png";
  GtkWidget *avatar = gtk_image_new_from_file(avatar_path);
  gtk_box_pack_start(GTK_BOX(content_box), avatar, FALSE, FALSE, 0);
  gtk_widget_set_size_request(avatar, 150, 150);
  gtk_widget_set_halign(avatar, GTK_ALIGN_CENTER);

  // Title: Username/Group name
  GtkWidget *title_label =
      gtk_label_new(is_group ? "Group Members" : "User Details");
  gtk_box_pack_start(GTK_BOX(content_box), title_label, FALSE, FALSE, 0);

  // Members info
  GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_box_pack_start(GTK_BOX(content_box), info_box, TRUE, TRUE, 10);

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
    gtk_box_pack_start(GTK_BOX(info_box), member_box, FALSE, FALSE, 5);

    // Username and status for all members in a group
    if (strcmp(username, Username) != 0 && !is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "Online" : "Offline");
      gtk_box_pack_start(GTK_BOX(member_box), username_label, TRUE, TRUE, 5);
      gtk_box_pack_start(GTK_BOX(member_box), status_label, TRUE, TRUE, 5);
    } else if (is_group) {
      GtkWidget *username_label = gtk_label_new(Username);
      GtkWidget *status_label =
          gtk_label_new(strcmp(status, "online") == 0 ? "Online" : "Offline");
      gtk_box_pack_start(GTK_BOX(member_box), username_label, TRUE, TRUE, 5);
      gtk_box_pack_start(GTK_BOX(member_box), status_label, TRUE, TRUE, 5);
    }

    if (!is_group && strcmp(username, Username) != 0) {
      // Additional details for private chats
      GtkWidget *details_grid = gtk_grid_new();
      gtk_grid_set_row_spacing(GTK_GRID(details_grid), 10);
      gtk_grid_set_column_spacing(GTK_GRID(details_grid), 15);
      gtk_box_pack_start(GTK_BOX(info_box), details_grid, FALSE, FALSE, 10);

      GtkWidget *name_label = gtk_label_new("Name:");
      GtkWidget *name_value = gtk_label_new(full_name);

      GtkWidget *group_label = gtk_label_new("Group:");
      GtkWidget *group_value = gtk_label_new(group);

      GtkWidget *role_label = gtk_label_new("Role:");
      GtkWidget *role_value = gtk_label_new(role);

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