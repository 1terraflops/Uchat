#include "uchat.h"

// A function that uses a separate thread to receive messages in parallel
void *receive_messages(void *arg) {
    (void)arg;
    char buffer[1024];

    while (1) {
        // Receive a message from the server
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);

        // Handle disconnection
        if (len == 0) {
            // Open database
            sqlite3 *uchat_database;
            if (sqlite3_open("uchat_database.db", &uchat_database) != SQLITE_OK) {
                fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(uchat_database));
                sqlite3_close(uchat_database);
                continue;
            }

            // Clear the chat display
            GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
            gtk_text_buffer_set_text(text_buffer, "", -1);

            // Notify the user about disconnection and attempt to reconnect
            show_server_offline_popup();
            reconnect_to_server();

            // Output chat history after reconnection
            display_chat_history(uchat_database, name, receiver);

            sqlite3_close(uchat_database);
            continue;
        }

        if (len < 0) {
            perror("recv failed");
            reconnect_to_server();
            continue;
        }

        // Null-terminate the received message
        buffer[len] = '\0';

        char *delimiter_pos = strchr(buffer, ':');
        if (delimiter_pos != NULL) {
            *delimiter_pos = '\0';  // Split sender and message
            char *sender = buffer;
            char *message = delimiter_pos + 1;

            // Trim leading whitespace from the message
            while (*message == ' ') {
                message++;
            }

            // Check if the sender is the active receiver
            if (strcmp(sender, receiver) != 0)
                continue;  // Ignore messages from other senders

            // Format the message with a timestamp
            time_t raw_time;
            struct tm *time_info;
            char time_str[50];
            time(&raw_time);
            time_info = localtime(&raw_time);
            strftime(time_str, sizeof(time_str), "%H:%M", time_info);

            char formatted_message[1100];
            snprintf(formatted_message, sizeof(formatted_message), "[%s] %s", time_str, message);

            if (formatted_message[strlen(formatted_message) - 1] == '\n') {
                formatted_message[strlen(formatted_message) - 1] = '\0';
            }

            // Display the message in the chat display
            GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
            GtkTextIter end;
            gtk_text_buffer_get_end_iter(text_buffer, &end);

            gtk_text_buffer_insert(text_buffer, &end, formatted_message, -1);
            gtk_text_buffer_insert(text_buffer, &end, "\n", -1);
        }
    }
}
