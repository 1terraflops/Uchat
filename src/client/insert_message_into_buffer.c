#include "uchat.h"

// A function that inserts messages into the buffer so the can be displayed
gboolean insert_message_into_buffer(gpointer data) {
    MessageData *message_data = (MessageData *)data;

    // Get the text buffer
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
    
    // Check received text
    if (!GTK_IS_TEXT_BUFFER(text_buffer)) {
        g_free(message_data);
        return FALSE;
    }

    // Get end position of the buffer
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(text_buffer, &end);

    // Include time and sender's username in the message
    char formatted_message[512];
    snprintf(formatted_message, sizeof(formatted_message), "[%s] %s: %s\n",
             message_data->time, message_data->username, message_data->message);
    
    // Insert string to display it
    gtk_text_buffer_insert(text_buffer, &end, formatted_message, -1);

    // Free the resources
    g_free(message_data);
    return FALSE;
}
