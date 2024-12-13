#include "uchat.h"

// Global variables for dynamic client management
client_info *active_clients = NULL;
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to add a new client
void add_client(const char *username, int socket_fd) {
    pthread_mutex_lock(&clients_mutex);
    client_info *temp = realloc(active_clients, sizeof(client_info) * (client_count + 1));
    if (!temp) {
        perror("realloc failed");
        pthread_mutex_unlock(&clients_mutex);
        return;
    }
    active_clients = temp;
    strncpy(active_clients[client_count].username, username, MAX_USERNAME_LENGTH - 1);
    active_clients[client_count].username[MAX_USERNAME_LENGTH - 1] = '\0';
    active_clients[client_count].socket_fd = socket_fd;
    client_count++;
    pthread_mutex_unlock(&clients_mutex);
}

// Function to remove a client
void remove_client(int socket_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (active_clients[i].socket_fd == socket_fd) {
            active_clients[i] = active_clients[client_count - 1];
            client_count--;
            client_info *temp = realloc(active_clients, sizeof(client_info) * client_count);
            if (temp || client_count == 0) {
                active_clients = temp;
            }
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to find a socket by username
int find_socket_by_username(const char *username) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (strcmp(active_clients[i].username, username) == 0) {
            int socket_fd = active_clients[i].socket_fd;
            pthread_mutex_unlock(&clients_mutex);
            return socket_fd;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1; // Not found
}

// Function to deliver a private message
void send_message(const char *message, const char *receiver_username) {
    int receiver_socket = find_socket_by_username(receiver_username);
    if (receiver_socket != -1) {
        if (send(receiver_socket, message, strlen(message), 0) < 0) {
            perror("send failed");
        }
    }
}

// Function to handle each client's communication
void *client_handler(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    char username[MAX_USERNAME_LENGTH] = {0};
    char receiver[MAX_USERNAME_LENGTH] = {0};
    char buffer[1024] = {0};

    // Receive client's username username
    if (read(client_socket, username, MAX_USERNAME_LENGTH) <= 0) {
        perror("Failed to read username");
        close(client_socket);
        return NULL;
    }

    // Add the client to the active clients list
    add_client(username, client_socket);

    // Loop where the client receives messages and sends them to the receiver
    while (1) {
        int read_value = read(client_socket, buffer, sizeof(buffer) - 1);
        if (read_value <= 0) {
            if (read_value < 0) perror("read failed");
            break;
        }

        buffer[read_value] = '\0';

        if (strncmp(buffer, "/change ", 8) == 0) {
            // Extract the new recipient from the buffer
            strncpy(receiver, buffer + 8, MAX_USERNAME_LENGTH - 1);
            receiver[MAX_USERNAME_LENGTH - 1] = '\0';
            printf("Recipient changed to: %s\n", receiver);
            continue;
        }

        // Deliver the message to the receiver
        char message[2048];
        snprintf(message, sizeof(message), "%s: %s\n", username, buffer);
        send_message(message, receiver);
    }


    // Remove the client on disconnect
    remove_client(client_socket);
    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Check if the provided port is valid
    if (argc != 2 || atoi(argv[1]) < 0) {
        fprintf(stderr, "usage: ./uchat <port>\n");
        exit(1);
    }

    // Create or/and open a database
    create_db();

    // Call a function that turns uchat_server into a daemon
    daemonize_server();

    // Declare server-related variables and an 'address' struct
    int port = atoi(argv[1]);
    int server_fd, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Set up the server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit(1);

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server_fd);
        exit(1);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 3) < 0) {
        close(server_fd);
        exit(1);
    }

    // Main loop
    while (1) {
        new_socket = malloc(sizeof(int));
        if (new_socket == NULL) {
            perror("malloc failed");
            continue;
        }
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            free(new_socket);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, (void *)new_socket) != 0) {
            perror("pthread_create failed");
            free(new_socket);
            close(*new_socket);
        }

        pthread_detach(thread_id);
    }
}
