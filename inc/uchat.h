#ifndef UCHAT_H
#define UCHAT_H

// Necessary includes
// Standard headers
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <syslog.h>
#include <stdio.h>
#include <time.h>

// 3rd-party library headers
#include "libmx.h"
#include "sqlite3.h"
#include <gtk/gtk.h>

#define MAX_USERNAME_LENGTH 30

// Struct to store client information
typedef struct client_info {
    char username[MAX_USERNAME_LENGTH];
    int socket_fd;
} client_info;

// Struct to store message information
typedef struct {
    const char *time;
    const char *username;
    const char *message;
} MessageData;

// Global variables
extern GtkWidget *main_window;
extern GtkWidget *chat_display;
extern GtkWidget *message_entry;
extern GtkWidget *search_entry;
extern GtkWidget *search_button;
extern GtkWidget *emoji_button;
extern GtkWidget *emoji_popover;
extern gboolean awaiting_receiver;
extern GtkWidget *label;
extern int sock, port;
extern char receiver[30], *name, *ip;

// Server functions
void daemonize_server(void);
void create_db(void);

// Client functions
void *receive_messages(void *arg);
char *ask_username(void);
char *ask_password(void);
char *insert_user_data(sqlite3 *uchat_database);
void display_chat_history(sqlite3 *uchat_database, const char *user, const char *receiver);
void save_message(const char *username, const char *receiver, const char *message);
char *ask_for_receiver(sqlite3 *uchat_database, char receiver[], const char username[]);
gboolean insert_message_into_buffer(gpointer data);
gboolean on_emoji_button_hover(void);
gboolean on_emoji_selected(GtkWidget *widget, gpointer emoji);
void on_send_button_clicked(void);
void on_search_button_clicked(void);
void reconnect_to_server(void);
void show_server_offline_popup(void);
void create_text_tags(GtkTextBuffer *buffer);
void show_welcome_popup(void);

#endif
