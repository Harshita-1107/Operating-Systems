#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <semaphore.h>

//Settings
#define INPUT_FILE "sample4_in.txt" //Input filename
#define MAX_CLIENTS 5               //The number of clients supported
#define RES_TYPE_COUNT 4            //The number of different resources available

///Macro Definiitons
#define MAX_BUFFER 100
#define REQUEST_CMD "RQ"
#define RELEASE_CMD "RL"
#define STATUS_CMD "Status"
#define RUN_CMD "Run"
#define EXIT_CMD "Exit"
#define MIN(a,b) ((a) < (b) ? a : b)
enum COMMANDS {RQ = 1, RL = 2, STATUS = 3, RUN = 4, EXIT = 5};
enum STATE {UNSAFE = 0, SAFE = 1};

//Function Declarations
void initMax();
void updateNeed();
void printMax();
void printNeed();
void printAllocation();
void printAvailable();
bool getCommand();
bool validateCommand(char *command);
int identifyCommand();
void processCommand(char *command);
void request(char *cmd[]);
void release(char *cmd[]);
void displayState();
void run();
bool getState();
void *threadRun(void *cid);

//Banker's Algorithm Data Structures
int maximum[MAX_CLIENTS][RES_TYPE_COUNT];
int available[RES_TYPE_COUNT];
int allocation[MAX_CLIENTS][RES_TYPE_COUNT];
int need[MAX_CLIENTS][RES_TYPE_COUNT];
int safe_sequence[MAX_CLIENTS] = {-1};
int state = SAFE;

int main(int argc, char *argv[]){

	if (argc < RES_TYPE_COUNT + 1){
		printf("Error: Initial resources not entered as arguments.\n");
		return 1;
	}

    for (int i = 0; i < RES_TYPE_COUNT; i++)
        if (!isdigit(*argv[1+i])){
            printf("Error: Invalid argument entered.\n");
            return 1;
        }

    printf("Number of customers: %d\n", MAX_CLIENTS);

    printf("Currently available resources: ");
    for (int i = 0; i < RES_TYPE_COUNT; i++) {       
        available[i] = atoi(argv[1+i]);
        printf("%d ", available[i]);        
    }

    initMax();
    updateNeed();
    printf("\nMaximum resources from file:\n");
    printMax();

    if (getState() == UNSAFE)
        printf("WARNING: The current state is unsafe.\n");

    while (getCommand());

    return 0;

}

void initMax() {

    FILE *fp = fopen(INPUT_FILE, "r");
    for (int i = 0; i < MAX_CLIENTS; i++)
        for (int j = 0; j < RES_TYPE_COUNT; j++)
            fscanf(fp, "%d,", &maximum[i][j]);
    fclose(fp);

}

void updateNeed() {

    for (int i = 0; i < MAX_CLIENTS; i++)
        for (int j = 0; j < RES_TYPE_COUNT; j++)
            need[i][j] = maximum[i][j] - allocation[i][j];

}

void printMax(){

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        for (int j = 0; j < RES_TYPE_COUNT; j++)
            printf("%d ", maximum[i][j]);
        printf("\n");
    }

}

void printNeed(){

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        for (int j = 0; j < RES_TYPE_COUNT; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }

}

void printAllocation(){

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        for (int j = 0; j < RES_TYPE_COUNT; j++)
            printf("%d ", allocation[i][j]);
        printf("\n");
    }

}

void printAvailable(){

    for (int i = 0; i < RES_TYPE_COUNT; i++)
        printf("%d ", available[i]);
    printf("\n");

}

bool getCommand(){

    char* input = calloc(MAX_BUFFER, sizeof(char));

    printf("Enter Command: ");
    while (fgets(input, MAX_BUFFER, stdin) != NULL){
        if (input[0] == '\n')
            continue;
        else
            break;
    }

    input[strcspn(input,"\n")] = 0;
    if (validateCommand(input)){
        switch(identifyCommand(input)){
            case RQ:
            case RL:
                processCommand(input);
                break;
            case STATUS:
                displayState();
                break;
            case RUN:
                run();
                break;
            case EXIT:
                printf("Program aborted.\n");
                exit(0);
                break;
            default:
                printf("Unknown error.\n");
                break;
        }
    }

    free(input);
    return true;

}

bool validateCommand(char *command){

    char temp[strlen(command)];
    strcpy(temp, command);
    char *token = strtok(temp, " ");

    if ( !(strcmp(token,REQUEST_CMD)) || (!(strcmp(token,RELEASE_CMD))) ){
        char cmd[3];
        strcpy(cmd, token);
        int arg_count = 0;
        while(token != NULL){
            if (isdigit(*token) > 0)
                arg_count++;
            token = strtok(NULL, " ");
        }
        if (arg_count != RES_TYPE_COUNT + 1){
            printf("%s: Invalid Usage.\n", cmd);
            printf("Usage: %s <client id> <res1> <res2> <res3>...\n", cmd);
            return false;
        }
        else
            return true;
    }
    else if ( !(strcmp(token,RUN_CMD)) || !(strcmp(token,STATUS_CMD)) || !(strcmp(token,EXIT_CMD)) )
        return true;

    printf("Invalid Command.\n");
    return false;

}

int identifyCommand(char *command){

    char temp[strlen(command)];
    strcpy(temp, command);
    char *token = strtok(temp, " ");
    if (!(strcmp(token,REQUEST_CMD)))
        return RQ;
    if (!(strcmp(token,RELEASE_CMD)))
        return RL;
    if (!(strcmp(token,RUN_CMD)))
        return RUN;
    if (!(strcmp(token,STATUS_CMD)))
        return STATUS;
    if (!(strcmp(token,EXIT_CMD)))
        return EXIT;

    return 0;

}

void processCommand(char *command){

    char *cmd[20];
    char temp[strlen(command)];
    strcpy(temp, command);
    char *token = strtok(temp, " ");
    int i = 0;
    while (token != NULL) {
        cmd[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    if (atoi(cmd[1]) >= MAX_CLIENTS){
        printf("%s: Invalid client number.\n", cmd[0]);
        return;
    }

    if (!(strcmp(cmd[0],REQUEST_CMD)))
        request(cmd);
    if (!(strcmp(cmd[0],RELEASE_CMD)))
        release(cmd);

}

void request(char *cmd[]){

    int client_id = atoi(cmd[1]);
    int res_request = 0;
    for (int i = 0; i < RES_TYPE_COUNT; i++){
        res_request = MIN(atoi((cmd[2+i])),need[client_id][i]);
        if (res_request > available[i]){
            printf("Resource request cannot be satisfied due to insufficient resources.\n");
            return;
        }

        if (res_request+allocation[client_id][i] > maximum[client_id][i]){
            printf("A client cannot request more resources than the maximum declared.\n");
            return;
        }
    }

    int temp[RES_TYPE_COUNT];
    for (int i = 0; i < RES_TYPE_COUNT; i++){
        res_request = MIN(atoi((cmd[2+i])),need[client_id][i]);
        temp[i] = res_request;
        available[i] -= res_request;
        need[client_id][i] -= res_request;
        allocation[client_id][i] += res_request;
    }

    if (getState() == UNSAFE){
        //printf("WARNING: The current state is unsafe.\n");
        for (int i = 0; i < RES_TYPE_COUNT; i++){
            available[i] += temp[i];
            need[client_id][i] += temp[i];
            allocation[client_id][i] -= temp[i];
        }
        getState();
        printf("Resource request has been denied due to safety concerns.\n");
    }
    else {
        printf("Resource request has been satisfied.\n");
    }


}

void release(char *cmd[]){

    int client_id = atoi(cmd[1]);
    int res_release = 0;
    int prev_state = state;

    // for (int i = 0; i < RES_TYPE_COUNT; i++)
    //     if (allocation[client_id][i] < atoi((cmd[2+i]))) {
    //         printf("A client cannot release more resources than they have allocated.\n");
    //         return;
    //     }

    for (int i = 0; i < RES_TYPE_COUNT; i++){
        res_release = MIN(atoi((cmd[2+i])),allocation[client_id][i]);
        available[i] += res_release;
        need[client_id][i] += res_release;
        allocation[client_id][i] -= res_release;
    }

    printf("Resources have been successfuly released.\n");

    if (getState() == UNSAFE)
        printf("WARNING: The current state is unsafe.\n");
    if (prev_state == UNSAFE && state == SAFE)
        printf("INFO: The current state is safe.\n");

}

void displayState(){

    printf("Maximum Resources:\n");
    printMax();
    printf("Allocated Resources:\n");
    printAllocation();
    printf("Need Resources:\n");
    printNeed();
    printf("Available Resources:\n");
    printAvailable();
    printf("State: %s\n", state ? "SAFE" : "UNSAFE");

}

void run(){

    if (state == SAFE){
        printf("Safe Sequence is: < ");
        for (int i = 0; i < MAX_CLIENTS; i++)
            printf("%d ", safe_sequence[i]);
        printf(">\n");
        printf("Executing threads:\n");

        pthread_t tid;
        for (int i = 0; i < MAX_CLIENTS; i++) {        
            pthread_create(&tid, NULL, threadRun, &safe_sequence[i]);
            pthread_join(tid, NULL);
        }
        printf("Thread execution complete.\n");
    }
    else
        printf("Current state is unsafe. Unable to proceed.\n");

}

bool getState() {

	state = SAFE;
	bool run[MAX_CLIENTS] = { false };
	
	int dummy_avail[RES_TYPE_COUNT];
	for (int c = 0; c < RES_TYPE_COUNT; c++)
		dummy_avail[c] = available[c];

	int seq_idx = 0;
	for (int k = 0; k < RES_TYPE_COUNT; k++)
		for (int i = 0; i < MAX_CLIENTS; i++)
			if (!run[i]) {
				bool flag = false;
				for (int j = 0; j < RES_TYPE_COUNT; j++)
					if (need[i][j] > dummy_avail[j]) {
						flag = true;
						break;
					}

				if (!flag) {
					safe_sequence[seq_idx++] = i;
					for (int h = 0; h < RES_TYPE_COUNT; h++)
						dummy_avail[h] += allocation[i][h];
					run[i] = true;
				}
			}

	for (int i = 0; i < MAX_CLIENTS; i++)
		if (!run[i])
			state = UNSAFE;

	return state;
	
}

void *threadRun(void *cid) {

	int *client_id = (int *)cid;
	printf("--> Client/Thread %d\n", *client_id);
	printf("\tAllocated Resources: ");
	for (int i = 0; i < RES_TYPE_COUNT; i++)
		printf("%d ", allocation[*client_id][i]);

	printf("\n");
	printf("\tNeeded: ");
	for (int i=0; i < RES_TYPE_COUNT; i++)
		printf("%d ", need[*client_id][i]);
	
	printf("\n");
	printf("\tAvailable: ");
	for (int i = 0; i < RES_TYPE_COUNT; i++)
		printf("%d ", available[i]);
	printf("\n");

	printf("\tThread has started.\n");
	sleep(1);
	printf("\tThread has finished.\n");
	printf("\tThread is releasing resources.\n");
	printf("\tNew Available: ");
	for (int i = 0; i < RES_TYPE_COUNT; i++){
		available[i] =  available[i] + allocation[*client_id][i];
		allocation[*client_id][i] = 0;
		printf("%d ", available[i]);
	}
	printf("\n");
	updateNeed();
	sleep(1);

	return NULL;
