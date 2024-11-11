#include <uchat.h>

void chat_creation(GtkWidget *create_chat_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_chat_data.username));

  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "action", "CREATE_CHAT");
  cJSON_AddStringToObject(json, "username", username);
  char *json_str = cJSON_Print(json);
  cJSON_Delete(json);
  send(main_page->sock, json_str, strlen(json_str), 0);
  g_print("Sent: %s\n", json_str);
  free(json_str);
  gtk_entry_set_text(GTK_ENTRY((*main_page).create_chat_data.username), "");
  gtk_label_set_text(GTK_LABEL((*main_page).create_chat_data.message), "");
}

void removing_user(GtkWidget *clicked_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  for (int i = 0; i < (*main_page).group_users_count; ++i) {
    if ((*main_page).group_users[i].button == clicked_button) {
      gtk_widget_destroy(gtk_widget_get_parent(clicked_button));
      (*main_page).group_users_count -= 1;
      int j;
      for (j = i; j < (*main_page).group_users_count; ++j) {
        (*main_page).group_users[j].button =
            (*main_page).group_users[j + 1].button;
        strcpy((*main_page).group_users[j].username,
               (*main_page).group_users[j + 1].username);
      }
      (*main_page).group_users[j].button = NULL;
      // strcpy((*main_page).group_users[i].username, "");
    }
  }
  gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
}

void adding_user(GtkWidget *add_user_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *username = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_group_data.username));

  if (check_form_data(username, NULL, (*main_page).create_group_data.message) ==
      1) {
    if ((*main_page).group_users_count == USERS_IN_GROUP_COUNT) {
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                         "Maximum number of members is reached");
    } else {
      cJSON *json = cJSON_CreateObject();
      cJSON_AddStringToObject(json, "action", "FIND_USER");
      cJSON_AddStringToObject(json, "username", username);
      char *json_str = cJSON_Print(json);
      cJSON_Delete(json);
      send(main_page->sock, json_str, strlen(json_str), 0);
      g_print("Sent: %s\n", json_str);
      free(json_str);

      gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username),
                         "");
      gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
    }
  }
}

void group_creation(GtkWidget *create_group_button, gpointer data) {
  t_main_page_data *main_page = (t_main_page_data *)data;
  char *name = (char *)gtk_entry_get_text(
      GTK_ENTRY((*main_page).create_group_data.name));

  if (strcmp(name, "") == 0)
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have a name");
  else if ((*main_page).group_users_count < 2)
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message),
                       "Group must have at least three members");
  else {
    g_print("Group %s with users ", name);
    char usernames[(*main_page).group_users_count][50];
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      g_print("%s%c", (*main_page).group_users[i].username,
              i == (*main_page).group_users_count - 1 ? '\n' : ' ');
      strcpy(usernames[i], (*main_page).group_users[i].username);
    }
    for (int i = 0; i < (*main_page).group_users_count; ++i) {
      gtk_widget_destroy(
          gtk_widget_get_parent((*main_page).group_users[i].button));
      strcpy((*main_page).group_users[i].username, "");
    }
    char *json_str =
        build_json_group_chat(name, usernames, (*main_page).group_users_count);
    send((*main_page).sock, json_str, strlen(json_str), 0);
    printf("Sent: %s\n", json_str);
    free(json_str);
    (*main_page).group_users_count = 0;
    //    gtk_stack_set_visible_child_name(GTK_STACK((*main_page).menu_stack),
    //                                   "chats_list");
    //(*main_page).menu_opened = -1;
    // gtk_stack_set_visible_child_name(GTK_STACK((*main_page).central_area_stack),
    //                               "chat");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.name), "");
    gtk_entry_set_text(GTK_ENTRY((*main_page).create_group_data.username), "");
    gtk_label_set_text(GTK_LABEL((*main_page).create_group_data.message), "");
  }
}

