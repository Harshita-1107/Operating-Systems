#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 
#define Mcust 25 

void *Bcust(void *num);
void *Barb(void *);
void cust_wait(int secs);

//Define the semaphores.
// waitingRoom Limits the # of Customers allowed  to enter the waiting room at one time.
sem_t Wroom;
// BarbChair ensures mutually exclusive access to the Barber chair.
sem_t BarbChair;
// BarbSleep is used to allow the Barb to sleep until a Customer arrives.
sem_t BarbSleep;
// cutseat is used to make the Bcust to wait until the Barb is done cutting his/her hair.
sem_t cutseat;
// variable to stop the Barb thread when all Customer have been serviced.
int allOver = 0;

int main(int argc, char *argv[])
{
    pthread_t btid;
    pthread_t tid[Mcust];
    int i, x, numBcusts, numChairs,perm=0; 
    int Number[Mcust];
    printf("Max Cust 25. Enter number of Customers < 25 and chairs.\n");
    scanf("%d",&x);
    numBcusts = x;
    scanf("%d",&x);
    numChairs = x;
    printf("Are Customers Coming to the Shop at Once.\n Yes(1) or No(0)\n");
    scanf("%d",&x);
    perm = x;
    if(perm==1 && numBcusts > numChairs){
        int leave = numBcusts - numChairs;
        printf("%d Customers will Leave the Shop as no place is Free",leave);
        numBcusts = numBcusts - leave;
    }
    else if (numBcusts > Mcust) {
       printf("Max customers entered are %d.\n", Mcust);
       system("PAUSE");   
       return 0;
    }
    printf("A solution to the sleeping Barb problem with semaphores.\n");
    for (i = 0; i < Mcust; i++) {
        Number[i] = i;
    }

    sem_init(&Wroom, 0, numChairs);
    sem_init(&BarbChair, 0, 1);
    sem_init(&BarbSleep, 0, 0);
    sem_init(&cutseat, 0, 0);
   
    // Create the Barb.
    pthread_create(&btid, NULL, Barb, NULL);
   
    // Create the Bcusts.
    for (i = 0; i < numBcusts; i++) {
        pthread_create(&tid[i], NULL, Bcust, (void *)&Number[i]);
    }

    for (i = 0; i < numBcusts; i++) {
        pthread_join(tid[i],NULL);
    }
    // When all of the Bcusts are finished, kill the Barb thread.
    allOver = 1;
    sem_post(&BarbSleep); // Wake the Barb so he will exit.
    pthread_join(btid,NULL);
    system("PAUSE");   
    return 0;
}

void *Bcust(void *number) {
     int num = *(int *)number; // Leave for the shop and take some random amount of  time to arrive.
     printf("Bcust %d leaving for Barb shop.\n", num);
     cust_wait(5);
     printf("Bcust %d arrived at Barb shop.\n", num); // Wait for space to open up in the waiting room...
     sem_wait(&Wroom);
     printf("Bcust %d entering waiting room.\n", num); // Wait for the Barb chair to become free.
     sem_wait(&BarbChair); // The chair is free so give up your place in the  waiting room.
     sem_post(&Wroom); // Wake up the Barb...
     printf("Bcust %d waking the Barb.\n", num);
     sem_post(&BarbSleep); // Wait for the Barb to finish cutting your hair.
     sem_wait(&cutseat); // Give up the chair.
     sem_post(&BarbChair);
     printf("Bcust %d leaving Barb shop.\n", num);
}

void *Barb(void *junk)
{ 
  while (!allOver) { // Sleep until someone arrives and wakes you..
    printf("The Barb is sleeping\n");
    sem_wait(&BarbSleep); // Skip this stuff at the end...
    if (!allOver)
    { // Take a random amount of time to cut the Bcust's hair.
     printf("The Barb is cutting hair\n");
     cust_wait(3);
     printf("The Barb has finished cutting hair.\n"); // Release the Bcust when done cutting...
     sem_post(&cutseat);
    }
    else {
         printf("The Day is Over!! Barber going to home.\n");
    }
   }
}
   
void cust_wait(int secs) {
     int len = 1;
     sleep(len);
}