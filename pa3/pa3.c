#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAXSEAT 256
#define MAXCLIENT 1024

typedef struct _query
{
    int user;
    int action;
    int data;
} query;

int main(int argc, char *argv[])
{
    int listenfd;

    int n, connfd, caddrlen;

    struct hostent *h;
    struct sockaddr_in saddr, caddr;
    query buf;
    int port = atoi(argv[1]);

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket() failed.\n");
        exit(1);
    }

    memset((char *)&saddr, 0, sizeof(saddr));

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
    {
        printf("bind() failed.\n");
        exit(2);
    }

    if (listen(listenfd, 5) < 0)
    {
        printf("listen() failed.\n");
        exit(3);
    }

    fd_set readset, copyset;
    FD_ZERO(&readset);
    FD_SET(listenfd, &readset);
    int fdmax = listenfd;
    int fdnum;

    int cnt_clinet = 0;
    int seat[MAXSEAT];
    memset(&seat, -1, sizeof(int) * MAXSEAT);

    while (1)
    {
        copyset = readset;

        if ((fdnum = select(fdmax + 1, &copyset, NULL, NULL, NULL)) < 0)
        {
            printf("select() failed\n");
            exit(0);
        }

        for (int i = 0; i < fdmax + 1; ++i)
        {
            if (FD_ISSET(i, &copyset))
            {
                if (i == listenfd)
                {
                    if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0)
                    {
                        printf("accept() failed.\n");
                        continue;
                    }
                    FD_SET(connfd, &readset);

                    n = read(connfd, &buf, sizeof(query));

                    // listen for connections from clients
                    // receive user name
                    // announce other user join

                    if (fdmax < connfd)
                    {
                        fdmax = connfd;
                    }
                }
                else
                {
                    // Send received messages to the clinet
                    // output the length of the input sentence on the clinet
                    // check how many users are left if any users have left
                }
            }
            else
            {
                if ((n = read(i, &buf, sizeof(query))) > 0)
                {
                }
                else
                {
                    FD_CLR(i, &readset);
                }
            }
        }
    }
    close(listenfd);
    // Log-in : Try to log in

    // Reserve : Try to reseve a seat

    // Check reservation Get the reseved seat number of a user

    // Cancel reservation Cancel reservatioin of a user

    // Log-out Log-out
}