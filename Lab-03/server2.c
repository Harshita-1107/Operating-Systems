#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "no port provided\n");
        exit(1);
    }

    int sockfd, newsockfd, portno, n;
    char buffer[255];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    pid_t child_pid;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("error opening socket\n");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("binding failed\n");
    }

    if (listen(sockfd, 5) != 0)
    {
        printf("Binding Error\n");
    }

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            error("error on accept\n");
        }
        printf("Connected with client socket %s %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        if (child_pid = fork() == 0)
        {
            close(sockfd);
            while (1)
            {
                bzero(buffer, 255);
                n = read(newsockfd, buffer, 255);
                if (n < 0)
                {
                    error("error on reading\n");
                }
                printf("Client socket %s %d sent message : %s", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), buffer);

            int num_a = (int)buffer[0] - 48;
            char operator= buffer[2];
            int num_b = (int)buffer[4] - 48;

            double result;
            if (operator== '+')
            {
                result = num_a + num_b;
            }

            if (operator== '-')
            {
                result = num_a - num_b;
            }

            if (operator== '*')
            {
                result = num_a * num_b;
            }

            if (operator== '/')
            {
                result = (num_a / (float)num_b);
            }

            int i = strncmp("-1", buffer, 2);
            if (i == 0)
            {
                result = (double)-999;
                n = write(newsockfd, &result, sizeof(double));
                printf("Disonnected with client socket %s %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                return 0;
            }
            printf("Sending reply : %f\n", result);

            n = write(newsockfd, &result, sizeof(double));
            if (n < 0)
            {
                error("error on writing\n");
            }
            }
        }
        else{
            close(newsockfd);
        }
        
    }
    
    close(sockfd);
    return 0;
}
