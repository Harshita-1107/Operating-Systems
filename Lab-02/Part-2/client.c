#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h>
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

void *send_announcement();
void *receive_announcement();

int i=0;
int queue_id;
char mess[MESSAGE_SIZE];        // an array for storing the entered message
announ announ_1;        // structure for storing the message to be sent
announ announ_2;        // structure for storing the message received from the queue

int main() {
        key_t key = ftok(".",'M');
        if(key == -1) {
                printf("client %d: Key Creating Error\n", getpid());
                exit(1);
        }

        queue_id = msgget(key, IPC_CREAT|0666);    // gaining access to the queue
        if(queue_id == -1) {
                perror("client: Error accessing message queue");
                exit(1);
        } else printf("client: Access to queue with ID '%d' \n", queue_id);
        
        pthread_t thread_1, thread_2;
        
        if(pthread_create(&thread_1, NULL, receive_announcement, NULL) == -1) {      // creating a thread to receive the message
                perror("client: Error creating thread to receive message\n");
                pthread_exit((void*)0);
        }

        if(pthread_create(&thread_2, NULL, send_announcement, NULL) == -1) {         // creating a thread for sending the message
                perror("client: Error creating thread for sending message\n");
                pthread_exit((void*)0);
        }

        if(pthread_join(thread_1, NULL) == -1) {
                perror("client: Error connecting the thread related to receiving the message\n");
                pthread_exit((void*)0);
        }

        if(pthread_join(thread_2, NULL) == -1) {
                perror("client: Error connecting a thread related to sending a message\n");
                pthread_exit((void*)0);
        }
}

void *send_announcement() {
        while(1) {
                pid_t client_pid = getpid();
                announ_1.type = 1;      // address
                announ_1.sender = client_pid;
                sleep(1);
                i=0;

                printf("\nclient %d: Enter the text to be sent: ", client_pid);
                while(1) {
                        mess[i]=getchar();
                        if((mess[i] == '\n') || (i > MESSAGE_SIZE - 1)) {
                                mess[i] = '\0';
                                break;
                        }
                        i++;
                }

                if(i > MESSAGE_SIZE - 1) {
                        printf("client %d ERROR: The given message is too long\n", client_pid);
                        pthread_exit(*send_announcement);
                }

                strcpy(announ_1.message, mess);

                if(msgsnd(queue_id, (announ*)&announ_1, strlen(announ_1.message) + 1 + sizeof(long), IPC_NOWAIT) == -1) {	// queue overflow
                        perror("client: There was an error in sending the message");
                        pthread_exit(*send_announcement);
                } else printf("client %d: A message has been sent to the server\n", client_pid);
        }
}

void *receive_announcement() {
        while(1) {
                pid_t client_pid = getpid();
                announ_2.type = client_pid;     // address

                memset(announ_2.message, 0, MESSAGE_SIZE);      // clearing the array

                if(msgrcv(queue_id, (announ*)&announ_2, MESSAGE_SIZE + sizeof(long), announ_2.type, 0) == -1) {       // receiving information 
                        perror("\nclient: Error while receiving a message from the server");
                        exit(1);
                } else printf("client %d: Message received: '%s' \n", client_pid, announ_2.message);
        }
}
