#include "uchat.h"

// A function that returns 'true' if the cursor hovers the emoji
gboolean on_emoji_button_hover(void) { 
    gtk_popover_popup(GTK_POPOVER(emoji_popover));
    return TRUE;
}

// A function that enables emojis to be sent
gboolean on_emoji_selected(GtkWidget *widget, gpointer emoji) {
    (void)widget;
    const char *emoji_text = (const char *)emoji;
    const char *current_text = gtk_entry_get_text(GTK_ENTRY(message_entry));
    char new_text[256];
    snprintf(new_text, sizeof(new_text), "%s%s", current_text, emoji_text);
    gtk_entry_set_text(GTK_ENTRY(message_entry), new_text);
    return TRUE;
}
