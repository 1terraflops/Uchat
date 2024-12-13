#include "uchat.h"

// A function that reconnects the client to the server
void reconnect_to_server(void) {
    // Close the old socket
    close(sock);

    // Create new socket to reconnect
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Set up new socket
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    // Try to reconnect
    while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        sleep(3);  // Wait before trying again

    // Send the username again after reconnection
    send(sock, name, strlen(name), 0);
}
