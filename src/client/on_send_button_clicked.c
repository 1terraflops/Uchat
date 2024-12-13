#include "uchat.h"

GtkTextTag *right_align_tag = NULL;

// A function that initializes the right_align tag
void create_right_align_tag(GtkTextBuffer *buffer) {
    // Check if the tag already exists
    if (!right_align_tag) {
        right_align_tag = gtk_text_buffer_create_tag(buffer, "right_align", "justification", GTK_JUSTIFY_RIGHT, NULL);
    }
}

// A function that sends a message when pressed on the 'send' button
void on_send_button_clicked(void) {
    // Get the text entered by the user in the message bar
    const char *message = gtk_entry_get_text(GTK_ENTRY(message_entry));

    // Check if the message is not empty and doesn't contain '/change'
    if (strlen(message) > 0 && strcmp(message, "/change") != 0) {
        // Set time variables
        time_t raw_time;
        struct tm *time_info;
        char time_str[50];

        // Get the current time
        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(time_str, sizeof(time_str), "%H:%M", time_info);

        // Set up text buffer
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Add sender's name, current time, and message
        char formatted_message[400];
        snprintf(formatted_message, sizeof(formatted_message), "%s [%s] \n", message, time_str);

        // Ensure the right-align tag exists
        create_right_align_tag(buffer);

        // Insert the formatted message with the right alignment tag
        gtk_text_buffer_insert_with_tags_by_name(buffer, &end, formatted_message, -1, "right_align", NULL);

        // Send the message to the server
        send(sock, message, strlen(message), 0);

        // Save the message to the database
        save_message(name, receiver, message);

        // Clear the text box
        gtk_entry_set_text(GTK_ENTRY(message_entry), "");
    }
}
