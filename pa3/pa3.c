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
#include <pthread.h>

#define MAXSEAT 256
#define MAXCLIENT 1024

pthread_attr_t attr;

typedef struct _query
{
    int user;
    int action;
    int data;
} query;

typedef struct _user_data
{
    int is_registered;
    int is_login;
    int passcode;
    pthread_mutex_t lock;
} user_data;

typedef struct _seat_data
{
    int reserve;
    pthread_mutex_t lock;
} seat_data;

typedef struct _thread_data
{
    int thread_fd;
    int thread_idx;
} thread_data;

seat_data seat_array[MAXSEAT];
user_data user_data_array[MAXCLIENT];
pthread_t client_thread_array[MAXCLIENT];
int valid_thread[MAXCLIENT];
int cnt_clinet = 0;

const int set_thread_idx()
{
    for (int i = 0; i < MAXCLIENT; ++i)
    {
        if (valid_thread[i] == -1)
        {
            return i;
        }
    }
    return -1;
}

void *thread(void *args)
{
    int fd = *((int *)args);
    // pthread_detach(pthread_self());
    free(args);
    query buf;
    int logined_user = -1;
    int n;

    while (1)
    {
        while (read(fd, &buf, sizeof(buf)) <= 0)
        {
            continue;
        }

        // Termination
        if (!(buf.user | buf.action | buf.data))
        {
            pthread_mutex_lock(&user_data_array[logined_user].lock);
            user_data_array[logined_user].is_login = 0;
            pthread_mutex_unlock(&user_data_array[logined_user].lock);

            int seat_reserve[MAXSEAT];
            for (int i = 0; i < MAXSEAT; ++i)
            {
                pthread_mutex_lock(&seat_array[i].lock);
                seat_reserve[i] = seat_array[i].reserve;
                pthread_mutex_unlock(&seat_array[i].lock);
            }

            write(fd, seat_reserve, sizeof(seat_reserve));
            close(fd);

            for (int i = 0; i < MAXCLIENT; ++i)
            {
                if (valid_thread[i] == fd)
                {
                    valid_thread[i] = -1;
                    break;
                }
            }
            return NULL;
        }

        pthread_mutex_lock(&user_data_array[buf.user].lock);

        int return_value = -1;
        switch (buf.action)
        {
        case 1: // Log-in : Try to log in
            if (!user_data_array[buf.user].is_registered)
            {
                user_data_array[buf.user].is_registered = 1;
                user_data_array[buf.user].passcode = buf.data;
                user_data_array[buf.user].is_login = 1;
                logined_user = buf.user;
                return_value = 1;
            }

            if (!user_data_array[buf.user].is_login && user_data_array[buf.user].passcode == buf.data)
            {
                user_data_array[buf.user].is_login = 1;
                logined_user = buf.user;
                return_value = 1;
            }

            break;

        case 2: // Reserve : Try to reseve a seat
            pthread_mutex_lock(&seat_array[buf.data].lock);
            return_value = -1;
            if (buf.data >= 0 && buf.data < MAXSEAT)
            {
                if (user_data_array[buf.user].is_login)
                {
                    if (seat_array[buf.data].reserve == -1)
                    {
                        seat_array[buf.data].reserve = buf.user;
                        return_value = buf.data;
                    }
                }
            }
            pthread_mutex_unlock(&seat_array[buf.data].lock);
            break;
        case 3: // Check reservation Get the reseved seat number of a user
            if (user_data_array[buf.user].is_login)
            {
                for (int i = 0; i < MAXSEAT; ++i)
                {
                    pthread_mutex_lock(&seat_array[i].lock);
                    if (seat_array[i].reserve == buf.user)
                    {
                        return_value = i;
                        pthread_mutex_unlock(&seat_array[i].lock);
                        break;
                    }
                    pthread_mutex_unlock(&seat_array[i].lock);
                }
            }
            break;
        case 4: // Cancel reservation Cancel reservatioin of a user
            pthread_mutex_lock(&seat_array[buf.data].lock);
            if (user_data_array[buf.user].is_login)
            {
                if (seat_array[buf.data].reserve == buf.user)
                {
                    seat_array[buf.data].reserve = -1;
                    return_value = buf.data;
                }
            }
            pthread_mutex_unlock(&seat_array[buf.data].lock);
            break;
        case 5: // Log-out Log-out
            if (user_data_array[buf.user].is_login)
            {
                user_data_array[buf.user].is_login = 0;
                return_value = 1;
            }

            break;
        default:
            break;
        }

        write(fd, &return_value, sizeof(return_value));

        pthread_mutex_unlock(&user_data_array[buf.user].lock);
    }
}

int main(int argc, char *argv[])
{
    int listenfd;

    int n, caddrlen;
    int *connfdp;

    struct hostent *h;
    struct sockaddr_in saddr, caddr;
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

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // init seat_array
    for (int i = 0; i < MAXSEAT; ++i)
    {
        seat_array[i].reserve = -1;
        pthread_mutex_init(&seat_array[i].lock, NULL);
    }

    // init user_data_array
    for (int i = 0; i < MAXCLIENT; ++i)
    {
        user_data_array[i].is_registered = user_data_array[i].is_login = user_data_array[i].passcode = 0;
        pthread_mutex_init(&user_data_array[i].lock, NULL);
    }

    for (int i = 0; i < MAXCLIENT; ++i)
    {
        valid_thread[i] = -1;
    }

    while (1)
    {
        connfdp = (int *)malloc(sizeof(int));
        // set accept
        if ((*connfdp = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0)
        {
            printf("accept() failed.\n");
            continue;
        }

        const int thread_idx = set_thread_idx();

        // make thread for user
        pthread_create(&client_thread_array[thread_idx], &attr, thread, (void *)connfdp);
        valid_thread[thread_idx] = *connfdp;
    }
    close(listenfd);
}