#include <uchat_server.h>

int validate_old_password(sqlite3 *db, const char *username, const char *old_pw) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT password_hash FROM users WHERE username = ?";
    char db_hashed_pw[SHA256_HASH_SIZE] = {0};
    char hashed_old_pw[SHA256_HASH_SIZE] = {0};

    // Hash the provided old password
    hash_password(old_pw, hashed_old_pw);

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1; // Error
    }

    // Bind the username to the query
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    // Execute the query
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Retrieve the hashed password from the database
        const char *stored_hashed_pw = (const char *)sqlite3_column_text(stmt, 0);
        if (stored_hashed_pw) {
            strncpy(db_hashed_pw, stored_hashed_pw, SHA256_HASH_SIZE - 1);
        }
    } else {
        fprintf(stderr, "No user found with username: %s\n", username);
        sqlite3_finalize(stmt);
        return 1; 
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    // Compare the hashes
    if (strcmp(db_hashed_pw, hashed_old_pw) == 0) {
        return 0;
    }

    return 1; // Password does not match
}

int handle_update_password(sqlite3 *db, Client *client, cJSON *json) {
    int user_id = get_user_id(db, client->username);
    if (user_id == -1) {
        fprintf(stderr, "Invalid session token.\n");
        return 1;
    }
    cJSON *old_pw_json = cJSON_GetObjectItem(json, "old_pw");
    cJSON *new_pw_json = cJSON_GetObjectItem(json, "new_pw");

    if (!old_pw_json || !cJSON_IsString(old_pw_json)) {
        fprintf(stderr, "Invalid or missing current password in JSON.\n");
        return 1; 
    }
    if (!new_pw_json || !cJSON_IsString(new_pw_json)) {
        fprintf(stderr, "Invalid or missing new password in JSON.\n");
        return 1;
    }

    const char *old_pw = old_pw_json->valuestring;
    const char *new_pw = new_pw_json->valuestring;
    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, "action", "UPDATE_PASSWORD");
    // Validate the old password
    if (validate_old_password(db, client->username, old_pw) != 0) {
        fprintf(stderr, "Current password is incorrect.\n");
         cJSON_AddStringToObject(response, "status", "FAILURE");
        cJSON_AddStringToObject(response, "error", "Incorrect current password.");
        send_json_responce_to_client(client, response);
        return 1;
    }

    // Hash the new password
    char hashed_new_pw[SHA256_HASH_SIZE];
    hash_password(new_pw, hashed_new_pw);

    const char *update_query = "UPDATE users SET password_hash = ? WHERE id = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, update_query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        return 1; // Failure
    }

    sqlite3_bind_text(stmt, 1, hashed_new_pw, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to update password: %s\n", sqlite3_errmsg(db));
        return 1; // Failure
    }

    
    cJSON_AddStringToObject(response, "status", "SUCCESS");
    send_json_responce_to_client(client, response);

    printf("Password updated successfully for user ID %d.\n", user_id);
    return 0;
}
