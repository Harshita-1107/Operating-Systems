#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_CLIENT_SUPPORTED 10

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

    fd_set readfds;

    int monitored_fd_set[10];

    int i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {
        monitored_fd_set[i] = -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("error opening socket\n");
    }

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

    i = 0;
    for (; i < MAX_CLIENT_SUPPORTED; i++)
    {
        if (monitored_fd_set[i] == -1)
        {
            monitored_fd_set[i] = sockfd;
            break;
        }
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    while (1)
    {
    
        // If monitored_fd_set's value is not -1 means that it is not a preassigned value and hence sets the file decriptor of it in readfd set.
        FD_ZERO(&readfds);
        i = 0;
        for (; i < MAX_CLIENT_SUPPORTED; i++)
        {
            if (monitored_fd_set[i] != -1)
            {
                FD_SET(monitored_fd_set[i], &readfds);
            }
        }
        
        // Select system call
        i = 0;
        int max = -999;

        for (; i < MAX_CLIENT_SUPPORTED; i++)
        {
            if (monitored_fd_set[i] > max)
            {
                max = monitored_fd_set[i];
            }
        }

        select(max + 1, &readfds, NULL, NULL, NULL);
        
        
        // check if the new connection is made or not
        if (FD_ISSET(sockfd, &readfds))
        {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
            if (newsockfd < 0)
            {
                error("error on accept\n");
            }
            printf("Connected with client socket %s %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            
            // assigning a new fd in the fd set if value found -1
            i = 0;
            for (; i < MAX_CLIENT_SUPPORTED; i++)
            {
                if (monitored_fd_set[i] == -1)
                {
                    monitored_fd_set[i] = newsockfd;
                    break;
                }
            }
        }
        
        // If not a new connection, then check on which client the message is arrived
        else
        {
            i = 0;
            for (; i < MAX_CLIENT_SUPPORTED; i++)
            {
                if (FD_ISSET(monitored_fd_set[i], &readfds))
                {
                    newsockfd = monitored_fd_set[i];
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

                    int j = strncmp("-1", buffer, 2);
                    if (j == 0)
                    {
                        result = (double)-999;
                        n = write(newsockfd, &result, sizeof(double));
                        printf("Disonnected with client socket %s %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                        
                        // deleting an existing fd in fd set if disconnected from server
                        close(newsockfd);
                        int k = 0;
                        for(; k < MAX_CLIENT_SUPPORTED; k++){
                            if(monitored_fd_set[k] == newsockfd)
                            {
                                monitored_fd_set[k] = -1;
                                break;
                            }
                        }
                        
                        break;
                    }
                    
                    
                    printf("Sending reply : %f\n", result);

                    n = write(newsockfd, &result, sizeof(double));
                    if (n < 0)
                    {
                        error("error on writing\n");
                    }
                }
            }
        }
    }
    return 0;
}

