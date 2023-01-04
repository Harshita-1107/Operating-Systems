#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define MESSAGE_SIZE 128 // max. size (length) of the message contained in the message

// structure of the message
typedef struct announcement {
        long type;
        long sender;
        char message[MESSAGE_SIZE];
} announ;

int queue_id;    // global variable to hold the queue ID

void signal_handler(int sig);

int main() {
        key_t key;
        announ announ_received;         // structure for storing the received message

        key=ftok(".",'M');
        if(key == -1) {
                printf("server: Key creation error\n");
                exit(1);
        }

        queue_id = msgget(key, IPC_CREAT|0666);    // gaining access to the queue
        if(queue_id == -1) {
                perror("server: Error accessing message queue\n");
                exit(1);
        } else printf("\nserver: Accessed queue with ID '%d'\n", queue_id);
        
        signal(SIGINT, signal_handler);     // installation of a new handler for the SIGINT signal (ctrl + c)
        printf("INFO: to exit the server press ^C\n");

        while(1) {
                printf("\nserver: Waiting for messages from clients ...\n");

                announ_received.type = 1; // server as the recipient of the message

                memset(announ_received.message, 0, MESSAGE_SIZE);       // clearing the array
                if(msgrcv(queue_id, (announ*)&announ_received, MESSAGE_SIZE + sizeof(long), announ_received.type, 0) == -1) {  // retrieving messages from the queue
                        perror("server: An error occurred while getting a message from the client\n");
                        signal_handler(SIGINT);
                } else printf("server: Received a message from the client about the process '%ld' containing the following content: '%s'\n", announ_received.sender, announ_received.message);

                int message_length = strlen(announ_received.message);

                for(int i = 0; i < message_length; i++) announ_received.message[i] = toupper(announ_received.message[i]);

                announ_received.type = announ_received.sender;

                printf("server: Sending a message to the client with the ID '%ld' with the following content:'%s'\n", announ_received.sender, announ_received.message);
                
                if(msgsnd(queue_id, (announ*)&announ_received, strlen(announ_received.message) + 1 + sizeof(long), IPC_NOWAIT) == -1) { // queue overflow
                        perror("server: An error occurred while sending the message");
                        signal_handler(SIGINT);
                }
        }
        return 0;
}

void signal_handler(int sig) {
        if(sig == SIGINT) {
                printf("\n\nserver: Server shutdown ...\n");

                if(queue_id == -1) {
                        perror("server: Error accessing message queue\n");
                        exit(1);
                }

                if(msgctl(queue_id, IPC_RMID, 0) == -1) {       // removing the queue
                        perror("server: Error deleting message queue: ");
                        exit(1);
                } else printf("the queue with ID '%d' was successfully removed\n", queue_id);
                
                printf("\nINFO: The server has ended successfully\n\n");
                exit(0);
        }
}
