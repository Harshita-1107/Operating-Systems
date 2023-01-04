#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

char lower(int x)
{
    return (x + 32);
}

int is_upper(char c)
{
    if (65 <= (int)c <= 90)
    {
        return 1;
    }
    return 0;
}

char upper(int x)
{
    return (x - 32);
}

int is_lower(char c)
{
    if (97 <= (int)c <= 122)
    {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int *mypipe;
    mypipe = (int *)malloc(sizeof(int) * 2);
    
    if (pipe(mypipe) == -1)
    {
        printf("An error occurred in creating a pipe \n");
        return 1;
    } //mypipe[0]-->read , mypipe[1]-->write
    
    //char *str;
    //str = (char *)malloc(sizeof(char) * 26);

    pid_t pid = fork();
    
    if (pid < 0)
    {
        printf("Error occurred at creating fork \n");
        return 2;
    }

    else if (pid == 0)
    {
        // child process
        char *str;
        str = (char *)malloc(sizeof(char) * 26);
        
        if (read(mypipe[0], str, 26) == -1)
        {
            return 5;
        }
        
        printf("received from parent --> %s \n", str);
        
        //str = read(mypipe[0], str, sizeof(str));
        
        int i=0;
        char ch;
        while (str[i] != '\0')
        {
            ch = str[i];
            if (ch >= 'A' && ch <= 'Z') //upper case to lower case
                str[i] = str[i] + 32;
            else if (ch >= 'a' && ch <= 'z') //lowercase to uppercase
                str[i] = str[i] - 32;
            i++;
        }

        if (write(mypipe[1], str, 26) == -1)
        {
            return 6;
        }
        printf("passing to parent --> %s \n", str);
        
        close(mypipe[1]);   //closing the writing end now
    }
    
    else if (pid > 0)
    {
        // parent process
        char *str;
        str = (char *)malloc(sizeof(char) * 26);
        
        str = argv[1];
        
        // scanf("%s ",str);
        
        if (write(mypipe[1], str, 26) == -1)
        {
            return 3;
        }
        printf("Passing to child --> %s \n", str);
        
        wait(NULL);    //waiting for child process to execute
        
        if (read(mypipe[0], str, 26) == -1)
        {
            return 4;
        }
        printf("result received from child finally --> %s \n", str);
        
        close(mypipe[0]);   //closing the reading end now
        
        //write(1, str, read(mypipe[0], str, sizeof(str)));
    }

    return 0;
}
