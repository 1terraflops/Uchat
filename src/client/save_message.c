#include "uchat.h"

// This function will save the sent message into the database
void save_message(const char *username, const char *receiver, const char *message) {
    // Prepare some variables
    sqlite3_stmt *stmt_user, *stmt_receiver, *stmt_insert;
    int user_id, receiver_id;

    // Open database
    sqlite3 *uchat_database;
    sqlite3_open("uchat_database.db", &uchat_database);

    // Write SQL statements to find the user_id and receiver_id
    const char *sql_find_user_id = "SELECT id FROM users WHERE username = ?";
    const char *sql_find_receiver_id = "SELECT id FROM users WHERE username = ?";
    const char *sql_insert_message = "INSERT INTO messages (user_id, receiver_id, message) VALUES (?, ?, ?)";

    // Find user_id
    // Prepare the statement
    if (sqlite3_prepare_v2(uchat_database, sql_find_user_id, -1, &stmt_user, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement to find user_id: %s\n", sqlite3_errmsg(uchat_database));
        return;
    }
    // Bind username to the statement
    sqlite3_bind_text(stmt_user, 1, username, -1, SQLITE_STATIC);

    // Execute the statement
    if (sqlite3_step(stmt_user) == SQLITE_ROW)
        user_id = sqlite3_column_int(stmt_user, 0);
    else {
        fprintf(stderr, "Error: Sender username not found: %s\n", username);
        sqlite3_finalize(stmt_user);
        return;
    }
    sqlite3_finalize(stmt_user);

    // Find receiver_id
    // Prepare the statement
    if (sqlite3_prepare_v2(uchat_database, sql_find_receiver_id, -1, &stmt_receiver, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement to find receiver_id: %s\n", sqlite3_errmsg(uchat_database));
        return;
    }
    // Bind username to the statement
    sqlite3_bind_text(stmt_receiver, 1, receiver, -1, SQLITE_STATIC);

    // Execute the statement
    if (sqlite3_step(stmt_receiver) == SQLITE_ROW)
        receiver_id = sqlite3_column_int(stmt_receiver, 0);
    else {
        fprintf(stderr, "Error: Receiver username not found: %s\n", receiver);
        sqlite3_finalize(stmt_receiver);
        return;
    }
    sqlite3_finalize(stmt_receiver);

    // Insert the message into the database
    if (sqlite3_prepare_v2(uchat_database, sql_insert_message, -1, &stmt_insert, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement to insert message: %s\n", sqlite3_errmsg(uchat_database));
        return;
    }

    // Bind data
    sqlite3_bind_int(stmt_insert, 1, user_id);
    sqlite3_bind_int(stmt_insert, 2, receiver_id);
    sqlite3_bind_text(stmt_insert, 3, message, -1, SQLITE_STATIC);

    // Execute SQL statement, check for errors
    if (sqlite3_step(stmt_insert) != SQLITE_DONE)
        fprintf(stderr, "Failed to insert message: %s\n", sqlite3_errmsg(uchat_database));

    // Free the resources and close database
    sqlite3_finalize(stmt_insert);
    sqlite3_close(uchat_database);
}
