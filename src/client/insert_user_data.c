#include "uchat.h"

// A function that displays an error to the user
void show_error_message(const char *message) {
    // Create a new pop-up
    GtkWidget *dialog = gtk_message_dialog_new(
        NULL,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_CLOSE,
        "%s",
        message
    );

    // Display it and wait for user to close
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog); // Close the dialog
}

// A function for button clicks
void on_button_clicked(GtkWidget *button, gpointer user_data) {
    int action = GPOINTER_TO_INT(user_data);
    GtkWidget *dialog = gtk_widget_get_toplevel(button);
    gtk_dialog_response(GTK_DIALOG(dialog), action);
}

// Get username and password from the user
int get_username_password_and_action(char **username, char **password) {
    // Prepare some variables
    GtkWidget *dialog, *content_area, *center_box, *username_entry, *password_entry, *button_box, *register_button, *login_button;
    GtkDialogFlags flags = GTK_DIALOG_MODAL;
    int action = -1; // -1 = no action, 0 = register, 1 = login

    // Create the dialog
    dialog = gtk_dialog_new_with_buttons(
        "Welcome!", NULL, flags, NULL, GTK_RESPONSE_NONE, NULL);
    gtk_widget_set_name(GTK_WIDGET(dialog), "auth_window");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Add styling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "src/client/style_auth_window.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Centering container
    center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(center_box, "auth_window_center_box");
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(content_area), center_box);

    // Username field
    username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");
    gtk_widget_set_name(username_entry, "auth_window_username_entry");
    gtk_box_pack_start(GTK_BOX(center_box), username_entry, FALSE, FALSE, 2);

    // Password field
    password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_widget_set_name(password_entry, "auth_window_password_entry");
    gtk_box_pack_start(GTK_BOX(center_box), password_entry, FALSE, FALSE, 2);

    // Button box
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(button_box, "auth_window_button_box");
    gtk_box_pack_start(GTK_BOX(center_box), button_box, FALSE, FALSE, 5);

    // Login button
    login_button = gtk_button_new_with_label("Login");
    gtk_widget_set_name(login_button, "auth_window_login_button");
    gtk_box_pack_start(GTK_BOX(button_box), login_button, TRUE, TRUE, 3);

    // Register button
    register_button = gtk_button_new_with_label("Register");
    gtk_widget_set_name(register_button, "auth_window_register_button");
    gtk_box_pack_start(GTK_BOX(button_box), register_button, TRUE, TRUE, 3);

    // Connect button signals
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(0));
    g_signal_connect(login_button, "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(1));

    gtk_widget_show_all(dialog);

    // Wait for user action
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // Handle user input if a valid action is selected
    if (response == GTK_RESPONSE_NONE) {
        *username = NULL;
        *password = NULL;
        action = -1;
    } else {
        *username = g_strdup(gtk_entry_get_text(GTK_ENTRY(username_entry)));
        *password = g_strdup(gtk_entry_get_text(GTK_ENTRY(password_entry)));

        // Validate input
        if (strlen(*username) < 4 || strlen(*password) < 4) {
            show_error_message("Username and password must be at least 4 characters long.");
            action = -1; // Reset action
        } else {
            action = GPOINTER_TO_INT(response);
        }
    }

    // Properly destroy the dialog
    gtk_widget_destroy(dialog);

    // Ensure GTK is properly cleaned up
    while (g_main_context_pending(NULL)) {
        g_main_context_iteration(NULL, FALSE);
    }

    return action;
}

// Register a new user
char *register_user(sqlite3 *uchat_database, const char *name, const char *password) {
    // Write SQL statement
    const char *sql_insert = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    int rc;

    // Prepare the statement
    rc = sqlite3_prepare_v2(uchat_database, sql_insert, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        show_error_message("Database error: Unable to prepare the SQL statement.");
        return NULL;
    }

    // Bind data to the statement
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("User registered successfully.\n");
        sqlite3_finalize(stmt);
        return g_strdup(name);
    }

    // Check for errors
    if (rc == SQLITE_CONSTRAINT)
        show_error_message("Username already exists. Please try a different username.");
    else
        show_error_message("Database error: Unable to register the user.");

    // Free the resources
    sqlite3_finalize(stmt);
    return NULL;
}

// Log in an existing user
char *sign_user_in(sqlite3 *uchat_database, const char *name, const char *password) {
    // Write the SQL statement
    const char *sql_query = "SELECT password FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;
    int rc;

    // Prepate the statement
    rc = sqlite3_prepare_v2(uchat_database, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        show_error_message("Database error: Unable to prepare the SQL statement.");
        return NULL;
    }

    // Bind the data to the statement
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

    // Check if the passwords match, notify the user in case if some errors
    if (rc == SQLITE_ROW) {
        const char *stored_password = (const char *)sqlite3_column_text(stmt, 0);
        if (strcmp(password, stored_password) == 0) {
            sqlite3_finalize(stmt);
            return g_strdup(name);
        } 
        else
            show_error_message("Incorrect password. Please try again.");
    } 
    else if (rc == SQLITE_DONE)
        show_error_message("Username not found. Please try again.");
    else
        show_error_message("Database error: Unable to fetch user data.");

    sqlite3_finalize(stmt);
    return NULL;
}

// Main authentication function
char *insert_user_data(sqlite3 *uchat_database) {
    // Prepare some variables
    char *name = NULL, *password = NULL;
    int action;

    // Ask user for correct data continuously
    while (1) {
        // Check whether the user wants to log in or register
        action = get_username_password_and_action(&name, &password);

        if (action == -1 || !name || !password) {
            printf("User cancelled input.\n");
            g_free(name);
            g_free(password);
            sqlite3_close(uchat_database);
            exit(1);
        }

        // Validate username and password length
        if (strlen(name) < 4 || strlen(password) < 4) {
            show_error_message("Username and password must be at least 4 characters long.");
            g_free(name);
            g_free(password);
            continue;  // Ask user for data again
        }

        char *result = NULL;
        if (action == 0) { // Register
            result = register_user(uchat_database, name, password);
        } else if (action == 1) { // Login
            result = sign_user_in(uchat_database, name, password);
        }

        // Free the resources
        g_free(name);
        g_free(password);

        if (result)
            return result;

        printf("Operation failed. Please try again.\n");
    }
}
