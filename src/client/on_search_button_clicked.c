#include "uchat.h"

// A function that searches for a user when pressed on the 'search' button
void on_search_button_clicked(void) {
    // Get the text entered by the user in the search bar
    const char *new_recipient = gtk_entry_get_text(GTK_ENTRY(search_entry));
    
    // Check if the search bar is empty
    if (strlen(new_recipient) > 0) {
        // Open database
        sqlite3 *uchat_database;
        sqlite3_open("uchat_database.db", &uchat_database);

        // A command to be sent to the server to switch users
        char change_command[50];
        snprintf(change_command, sizeof(change_command), "/change %s", new_recipient);
        
        // Send the command to the server
        send(sock, change_command, strlen(change_command), 0);

        // Clear the window
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_display));
        gtk_text_buffer_set_text(buffer, "", -1);

        // Clear the label
        gtk_label_set_text(GTK_LABEL(label), "");

        char temp[100];
        sprintf(temp, "Chatting with %s", new_recipient);

        // Output new recepient
        gtk_label_set_text(GTK_LABEL(label), temp);

        // Display chat history again
        display_chat_history(uchat_database, name, new_recipient);

        // Update the current receiver
        strncpy(receiver, new_recipient, sizeof(receiver) - 1);
        receiver[sizeof(receiver) - 1] = '\0';

        // Clear the search entry field
        gtk_entry_set_text(GTK_ENTRY(search_entry), "");

        // Close database
        sqlite3_close(uchat_database);
    }
}
