#include "uchat.h"

// A variable that check if alignment already happened
static gboolean tags_created = FALSE;

// This function displays the chat's history for a private conversation
void display_chat_history(sqlite3 *uchat_database, const char *user, const char *receiver) {
    // Prepare some variables
    sqlite3_stmt *stmt_user, *stmt_receiver, *stmt_history;
    int user_id, receiver_id;

    // Write SQL statements
    const char *sql_find_user_id = "SELECT id FROM users WHERE username = ?";
    const char *sql_find_receiver_id = "SELECT id FROM users WHERE username = ?";
    const char *sql_display_history =
        "SELECT users.username, messages.message, messages.time "
        "FROM messages "
        "INNER JOIN users ON users.id = messages.user_id "
        "WHERE (messages.user_id = ? AND messages.receiver_id = ?) "
        "   OR (messages.user_id = ? AND messages.receiver_id = ?) "
        "ORDER BY messages.time";

    // Find user_id
    if (sqlite3_prepare_v2(uchat_database, sql_find_user_id, -1, &stmt_user, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt_user, 1, user, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt_user) == SQLITE_ROW) {
            user_id = sqlite3_column_int(stmt_user, 0);
        }
        sqlite3_finalize(stmt_user);
    }

    // Find receiver_id
    if (sqlite3_prepare_v2(uchat_database, sql_find_receiver_id, -1, &stmt_receiver, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt_receiver, 1, receiver, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt_receiver) == SQLITE_ROW) {
            receiver_id = sqlite3_column_int(stmt_receiver, 0);
        }
        sqlite3_finalize(stmt_receiver);
    }

    // Get the text buffer for the chat display
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));

    // Create the tags if they haven't been created yet
    if (!tags_created) {
        gtk_text_buffer_create_tag(buffer, "user_right_align", "justification", GTK_JUSTIFY_RIGHT, NULL);
        gtk_text_buffer_create_tag(buffer, "receiver_left_align", "justification", GTK_JUSTIFY_LEFT, NULL);
        tags_created = TRUE;  // Set the flag to true, indicating tags have been created
    }

    // Display chat history
    if (sqlite3_prepare_v2(uchat_database, sql_display_history, -1, &stmt_history, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt_history, 1, user_id);
        sqlite3_bind_int(stmt_history, 2, receiver_id);
        sqlite3_bind_int(stmt_history, 3, receiver_id);
        sqlite3_bind_int(stmt_history, 4, user_id);

        // Set up buffer
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Retrieve message history from the database
        while (sqlite3_step(stmt_history) == SQLITE_ROW) {
            const char *username = (const char *)sqlite3_column_text(stmt_history, 0);
            const char *message = (const char *)sqlite3_column_text(stmt_history, 1);
            const char *time = (const char *)sqlite3_column_text(stmt_history, 2);

            // Parse current time
            struct tm tm_info;
            strptime(time, "%Y-%m-%d %H:%M:%S", &tm_info);

            // Format it to only show hours and minutes
            char formatted_time[6];
            strftime(formatted_time, sizeof(formatted_time), "%H:%M", &tm_info);

            // Allocate memory for the data
            MessageData *message_data = g_malloc(sizeof(MessageData));
            message_data->time = g_strdup(formatted_time);
            message_data->username = g_strdup(username);
            message_data->message = g_strdup(message);

            // Check if the message was sent by the user or receiver
            if (strcmp(username, user) == 0) {
                // Align to the right
                gtk_text_buffer_insert_with_tags_by_name(buffer, &end, message_data->message, -1, "user_right_align", NULL);
                char formatted_message[400];
                snprintf(formatted_message, sizeof(formatted_message), " [%s]\n", message_data->time);
                gtk_text_buffer_insert(buffer, &end, formatted_message, -1);
            } else {
                // Align to the left
                char formatted_message[400];
                snprintf(formatted_message, sizeof(formatted_message), "[%s] %s\n", message_data->time, message_data->message);
                gtk_text_buffer_insert_with_tags_by_name(buffer, &end, formatted_message, -1, "receiver_left_align", NULL);
            }

            // Free the message data
            g_free(message_data);
        }
        sqlite3_finalize(stmt_history);
    }
}
