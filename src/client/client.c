#include "uchat.h"

// Global variables
GtkWidget *main_window;
GtkWidget *chat_display;
GtkWidget *message_entry;
GtkWidget *search_entry;
GtkWidget *search_button;
GtkWidget *emoji_button;
GtkWidget *emoji_popover;
GtkWidget *label;
gboolean awaiting_receiver = TRUE;
int sock, port;
char receiver[30], *name = NULL, *ip = NULL;

// Main function of the client
int main(int argc, char *argv[]) {
    // Validate arguments
    if (argc != 3 || atoi(argv[2]) < 0) {
        fprintf(stderr, "usage: ./uchat <IP> <port>\n");
        exit(1);
    }

    // Store the arguments
    ip = strdup(argv[1]);
    port = atoi(argv[2]);

    // Initialize gtk
    gtk_init(&argc, &argv);

    // Create socket and connect to the server
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Open the database
    sqlite3 *uchat_database;
    int rc = sqlite3_open("uchat_database.db", &uchat_database);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(uchat_database));
        sqlite3_close(uchat_database);
        exit(1);
    }

    // Insert user data into the database
    name = insert_user_data(uchat_database);
    send(sock, name, strlen(name), 0);

    // Start the message reception thread
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, NULL) != 0) {
        perror("pthread_create failed");
        close(sock);
        sqlite3_close(uchat_database);
        free(name);
        free(ip);
        exit(1);
    }

    // Load CSS styles
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "src/client/style_client.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
    g_object_unref(css_provider);

    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Uchat");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 950, 600);
    gtk_widget_set_name(main_window, "main-window");

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    // Top panel
    GtkWidget *top_panel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), top_panel, FALSE, FALSE, 5);
    gtk_widget_set_name(top_panel, "top-panel");

    // Prepare custom 'hello' sign
    char temp[50];
    sprintf(temp, "Hey, %s!", name);

    // Create label and apply styles
    label = gtk_label_new(temp);
    gtk_widget_set_name(label, "label");
    gtk_box_pack_start(GTK_BOX(top_panel), label, FALSE, FALSE, 5);

    // Construct search bar
    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search...");
    gtk_box_pack_start(GTK_BOX(top_panel), search_entry, TRUE, TRUE, 5);
    gtk_widget_set_name(search_entry, "search-entry");

    // Search button
    search_button = gtk_button_new_with_label("Search");
    gtk_box_pack_start(GTK_BOX(top_panel), search_button, FALSE, FALSE, 5);
    gtk_widget_set_name(search_button, "search-button");

    // Chat display and scroll
    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_name(scroll_window, "scroll-window");

    // Create the chat display and add it to the scroll window
    chat_display = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_display), FALSE);
    gtk_container_add(GTK_CONTAINER(scroll_window), chat_display);
    gtk_widget_set_name(chat_display, "chat-display");

    // Add the scroll window to the main box
    gtk_box_pack_start(GTK_BOX(main_box), scroll_window, TRUE, TRUE, 5);

    // Bottom panel
    GtkWidget *bottom_panel = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box), bottom_panel, FALSE, FALSE, 5);
    gtk_widget_set_name(bottom_panel, "bottom-panel");

    // Build widget pannel
    emoji_button = gtk_button_new_with_label("😊");
    gtk_box_pack_start(GTK_BOX(bottom_panel), emoji_button, FALSE, FALSE, 5);
    gtk_widget_set_name(emoji_button, "emoji-button");

    emoji_popover = gtk_popover_new(emoji_button);
    GtkWidget *emoji_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(emoji_popover), emoji_grid);

    const char *emojis[] = { "😊", "😂", "😍", "😢", "😡",
                             "😜", "👍", "👎", "🥰", "🤯",
                             "🥲", "😇", "🤔", "😬", "🙌",
                             "👀", "😱", "😤", "🖕", "😭" };
    int columns = 4;
    for (int i = 0; i < 20; i++) {
        GtkWidget *emoji_button = gtk_button_new();
        gtk_widget_set_size_request(emoji_button, 28, 28);
        gtk_button_set_relief(GTK_BUTTON(emoji_button), GTK_RELIEF_NONE);
        GtkWidget *emoji_label = gtk_label_new(emojis[i]);
        gtk_container_add(GTK_CONTAINER(emoji_button), emoji_label);
        gtk_grid_attach(GTK_GRID(emoji_grid), emoji_button, i % columns, i / columns, 1, 1);
        g_signal_connect(emoji_button, "clicked", G_CALLBACK(on_emoji_selected), (gpointer)emojis[i]);
    }

    gtk_widget_show_all(emoji_grid);

    // Configure message entry bar
    message_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(bottom_panel), message_entry, TRUE, TRUE, 5);
    gtk_widget_set_name(message_entry, "message-entry");

    // Build button to send messages
    GtkWidget *send_button = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(bottom_panel), send_button, FALSE, FALSE, 5);
    gtk_widget_set_name(send_button, "send-button");

    // Connect signals
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), NULL);
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), NULL);
    g_signal_connect(emoji_button, "enter-notify-event", G_CALLBACK(on_emoji_button_hover), NULL);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(main_window);

    show_welcome_popup();

    // Launch the window
    gtk_main();

    // Clean up
    close(sock);
    sqlite3_close(uchat_database);
    free(name);
    free(ip);

    return 0;
}
