#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_SIZE 256
#define SERVER_QUEUE_KEY 12345

// Structure for message queue
struct message {
    long msg_type;
    char msg_text[MSG_SIZE];
};

void start_server() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process becomes the server
        execl("./game_server", "./game_server", NULL);
        perror("Error starting server");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        printf("Server started with PID: %d\n", pid);
    } else {
        perror("Fork failed");
    }
}

void connect_to_server() {
    int msgid = msgget(SERVER_QUEUE_KEY, 0666);
    if (msgid < 0) {
        perror("Failed to connect to server");
        return;
    }

    printf("Connected to the server.\n");

    struct message msg;
    msg.msg_type = 1;
    snprintf(msg.msg_text, MSG_SIZE, "New player connected!");

    if (msgsnd(msgid, &msg, sizeof(msg.msg_text), 0) == -1) {
        perror("Failed to send message to server");
    } else {
        printf("Message sent to server: %s\n", msg.msg_text);
    }
}

int main() {
    int choice;

    printf("1. Start new server\n");
    printf("2. Connect to existing server\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            start_server();
            break;
        case 2:
            connect_to_server();
            break;
        default:
            printf("Invalid choice.\n");
    }

    return 0;
}
