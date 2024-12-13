#include "uchat.h"

// This function creates and/or opens a database
void create_db(void) {
    // Create and open a database
    sqlite3 *uchat_database;
    int rc = sqlite3_open("uchat_database.db", &uchat_database);
    char *err_msg = 0;

    // Check for errors
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(uchat_database));
        sqlite3_close(uchat_database);
        exit(1);
    }

    // Create a new table to store users
    const char *sql_create = "CREATE TABLE IF NOT EXISTS users("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "username TEXT UNIQUE NOT NULL,"
                                "password TEXT NOT NULL"
                             ")";

    // Execute the statement
    rc = sqlite3_exec(uchat_database, sql_create, 0, 0, &err_msg);

    // Check for errors
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        free(err_msg);
        sqlite3_close(uchat_database);
        exit(1);
    }

    // Create a new table to store messages
    const char *sql_create_messages = "CREATE TABLE IF NOT EXISTS messages ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                        "user_id INTEGER NOT NULL, "
                                        "receiver_id INTEGER NOT NULL, "
                                        "message TEXT, "
                                        "time DATETIME DEFAULT CURRENT_TIMESTAMP, "
                                        "FOREIGN KEY (user_id) REFERENCES users(id)"
                                        "FOREIGN KEY (receiver_id) REFERENCES users(id)"
                                      ")";


    // Execute the statement
    rc = sqlite3_exec(uchat_database, sql_create_messages, 0, 0, &err_msg);

    // Check for errors
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        free(err_msg);
        sqlite3_close(uchat_database);
        exit(1);
    }

    // Close database
    sqlite3_close(uchat_database);
}
