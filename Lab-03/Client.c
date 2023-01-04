#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_INPUT_SIZE 256
int main(int argc, char *argv[])
{
    int sockfd, portnum, n;
    struct sockaddr_in server_addr;
    char inputbuf[MAX_INPUT_SIZE];
    if (argc < 3)
    {
        fprintf(stderr, "Server IP address and port no. both are required\n");
        exit(0);
    }
    portnum = atoi(argv[2]);
    
    /* Fill in server address */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (!inet_aton(argv[1], &server_addr.sin_addr))
    {
        fprintf(stderr, "ERROR invalid server IP address\n");
        exit(1);
    }
    server_addr.sin_port = htons(portnum);
    
    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
    }
    
    /* Connect to server */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {

        fprintf(stderr, "error in connection !!!\n");
        exit(1);
    }
    printf("Connected to server\n");
    printf("If you want to exit, type -1\n");
    
    while(1)
    {
        
        printf("Please enter the message to the server: ");
        bzero(inputbuf, MAX_INPUT_SIZE);
        fgets(inputbuf, MAX_INPUT_SIZE - 1, stdin);
        
        n = write(sockfd, inputbuf, strlen(inputbuf));
        if (inputbuf == "-1"){
            break;
        }
        if (n < 0)
        {
            fprintf(stderr, "ERROR writing to socket\n");
            exit(1);
        }
        
        bzero(inputbuf, MAX_INPUT_SIZE);
        double result;
        n = read(sockfd, &result, sizeof(double));
        n = read(sockfd, inputbuf, (MAX_INPUT_SIZE - 1));

        if (n < 0)
        {
            fprintf(stderr, "ERROR reading from socket\n");
            exit(1);
        }
        if (result == (double)-999){
            printf("Server Disconnected\n");
            break;
        }
        else{
            printf("Server replied: %f\n", result);
        }
    }
    close(sockfd);
    return 0;
}
