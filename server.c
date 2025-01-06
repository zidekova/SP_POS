#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#define MSG_SIZE 256
#define SERVER_QUEUE_KEY 12345

// Struktura správy pre message queue
struct message {
    long msg_type;
    char msg_text[MSG_SIZE];
};

// Funkcia, ktorá spracováva správy od klienta
void *handle_client(void *arg) {
    int msgid = *((int *)arg);
    struct message msg;

    while (1) {
        // Čaká na príchod správy od klienta
        if (msgrcv(msgid, &msg, sizeof(msg.msg_text), 0, 0) == -1) {
            perror("Chyba pri prijímaní správy");
            pthread_exit(NULL);
        } else {
            printf("Prijatá správa: %s\n", msg.msg_text);

            // Ak dostane správu "KONIEC", ukončí vlákno
            if (strcmp(msg.msg_text, "KONIEC") == 0) {
                printf("Klient sa odpojil.\n");
                pthread_exit(NULL);
            }
        }
    }
}

int main() {
    int msgid;

    // Vytvorenie fronty správ
    msgid = msgget(SERVER_QUEUE_KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("Chyba pri vytváraní fronty správ");
        exit(EXIT_FAILURE);
    }

    printf("Server beží. Čakám na pripojenie hráčov...\n");

    while (1) {
        // Vytvára nové vlákno pre každého klienta
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, &msgid) != 0) {
            perror("Chyba pri vytváraní vlákna");
        } else {
            printf("Nový klient sa pripojil. Vlákno ID: %ld\n", thread_id);
        }
    }

    // Odstránenie fronty správ pri ukončení servera
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("Chyba pri odstraňovaní fronty správ");
    }

    return 0;
}
