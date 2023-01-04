#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXSize 1024 
#define HISSize 10

void execute(char** args);
char** ParseCommandLine(char* InputCommand);


int main(int argc, char *argv[], char * envp[]){
	int i = 0; // loop count for history array
	int k;
	int current = 0;// 'current' mark the index of history array
	int count = 0;
	char** args;//the array for all the tokens in the command line
	char directory[100];//store the cd directories into the string
	char InputCommand[MAXSize]; 
	char* hist_arr[HISSize]; // the array to store history

	for(i = 0; i < HISSize; i++)// initialize the history array with NULL
			hist_arr[i] = NULL;
			
        printf("\n\n******************"
		"************************");
		printf("\n\n\t***This is MY SHELL***");
		printf("\n\n\t*Type help to know more*");
                printf("\n\n*******************"
                "***********************\n\n");

	for(;;){//ifinite loop
		//title for each command line
             
		printf("Harshita@my_shell:~$ ");

		fgets(InputCommand, sizeof(InputCommand), stdin); // store the command line from user to the array
		
		free(hist_arr[current]); // free the history array

		hist_arr[current] = strdup(InputCommand);
		current = (current +1) % HISSize;

                //If the internal command line is "help" the shell show below output
		if(strcmp(InputCommand, "help\n") == 0){
		        printf("\n***WELCCOMME TO MY SHELL HELP***"
		            "\nCopyright @Harshita Gupta"
		            "\nList of Commands supported:"
		            "\n ls: lists files in current directory"
		            "\n clear: clear the screen"
		            "\n env: displays all environment parameters"
		            "\n pwd: prints the current directory"
		            "\n history: displays the commands the user ran"
		            "\n help: to get info shell and supported commands"
		            "\n cd <dir>"
		            "\n mkdir <dir>: creates a directory called 'dir'"
		            "\n rmdir <dir>: removes the directory called 'dir'"
		            "\n exit: exits the shell"
		            "\n pipe handling"
                            "\n improper space handling\n\n");
		}

		//If the internal command line is "exit", the shell will exit
		if(strcmp(InputCommand, "exit\n") == 0){
			printf("Goodbye! my_shell exit!\n\n");
			exit(1);
		}
		
		//for internal "cd" command
		args = ParseCommandLine(InputCommand); //split the command line into each token

		char* cd_command;
		char* command; // 'command' store the command without \n 

		cd_command = strtok(InputCommand, " ");
		command = InputCommand;

		if(strcmp(cd_command, "cd") == 0){
			args[1] = strtok(args[1], "\n");
			if(chdir(args[1]) == -1){
				printf("***ERROR: can not find the file or directory.\n");
				free(hist_arr[current]);
				hist_arr[current] = strdup(InputCommand);			
			}
		}
		
		else if(strcmp(command, "pwd\n") == 0){// internal pwd command
			getcwd(directory, sizeof(directory));
			if(directory != NULL)
				printf("Directory:  %s\n", directory);
			else
				printf("***ERROR: you can not find the directory!\n");
		}
		
		else if(strcmp(command, "mkdir\n") == 0){// internal mkdir command
			char* dirname;
			printf("Enter dir/file:");
			scanf("%s", dirname);
			mkdir(dirname,0777);
  
		}
		
		else if(strcmp(command, "rmdir\n") == 0){// internal rmdir command
			char* dirname;
			printf("Enter dir/file:");
			scanf("%s", dirname);
			rmdir(dirname);
  
		}
		
		else if(strcmp(command, "clear\n") == 0){// internal clear command
			system("clear");
		}
		
		else if(strcmp(command, "env\n") == 0){// internal env command	
			int i;
			for(i=0; envp[i] != NULL; i++)
			    printf("\n%s", envp[i]);
			    
			getchar();
		}
		
		else if(strcmp(command, "history\n") == 0){ // internal history command
			k = current;
			int his_count = 1;
			printf("\n");
			do{
				if(hist_arr[k] != NULL && strcmp(hist_arr[k], "\n") != 0)
					printf("%4d %s", his_count++, hist_arr[k]);
				k = (k+1) % HISSize;
			}
			while (k != current);
			printf("\n");
		}
		
		if(strcmp(command, "pwd\n") != 0 && strcmp(cd_command, "cd") != 0 &&
			strcmp(command, "history\n") != 0 && strcmp(command, "ls\n") != 0 && 
			strcmp(command, "help\n") != 0 && strcmp(command, "clear\n") != 0 && 
			strcmp(command, "env\n") != 0 && strcmp(command, "mkdir\n") != 0 &&
			strcmp(command, "rmdir\n") != 0 && strcmp(command, "cat\n") != 0 && 
			strcmp(command, "\n"))	
			printf("***ERROR: can not find the command.\n");

		//external command
		pid_t pid;
		int status;
		pid = fork();

		if(pid < 0){
			printf("***ERROR: forking child process failed.\n");
			exit(1);
		}
		else if(pid == 0){//child process
			if (strcmp(command, "cat") == 0){// cat command
				memmove(command, command+4, strlen(command+4)+1);
				command = strtok(command, "\n");
				char* argv[] = {"cat", command, 0};
				execvp(argv[0], argv);//use execvp to run 'cat'
			}
			else if(strcmp(command, "ls\n") == 0){// ls command
				char* argm[] = {"ls", 0};
				execvp(argm[0], argm);//use execvp to run 'ls'
			}
			
			exit(1);
		}
		else{//parent process
			while(wait(&status) != pid);
		}
	}
	return 0;
}

char** ParseCommandLine(char* InputCommand){
 	char** arr_tokens = malloc(MAXSize * sizeof(char*)); // allocate the space for each tokens in the array
 	char* token; //each token in each command line
 	token = strtok(InputCommand, " ");
 	int index = 0; //index in arr_tokens array

 	while(token != NULL){
 		arr_tokens[index++] = token;//store each token in array
 		token = strtok(NULL, " ");
 	}
 	arr_tokens[index] = NULL;
 	return arr_tokens;
}
