#include <uchat.h>

// A function that makes a pop-up to notify a user about disconnection
void show_welcome_popup(void) {
    // Create a pop-up
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "Welcome! Use search to start new chats!");
    // Set the title of the pop-up
    gtk_window_set_title(GTK_WINDOW(dialog), "Welcome!");

    // Connect the signal and run the pop-up
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_dialog_run(GTK_DIALOG(dialog));
}
