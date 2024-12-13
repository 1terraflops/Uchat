#include "uchat.h"

// A function that makes uchat_server a daemon
void daemonize_server(void) {
    pid_t pid, sid;
    pid = fork(); // Fork the process

    if (pid > 0){ // Exit the parent process
        printf("uchat_server has started with pid: %d\n", pid);
        exit(0);
    }

    sid = setsid(); // Create a new session
    if (sid < 0) exit(1);

    chdir("/"); // Change the working directory
    umask(0); // Set file permissions mask

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Report into the system journal
    openlog("uchat_server", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_NOTICE, "uchat_server has started with pid: %d\n", pid);
}
