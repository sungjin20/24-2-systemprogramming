/*---------------------------------------------------------------------------*/
/* server.c                                                                  */
/* Author: Junghan Yoon, KyoungSoo Park                                      */
/* Modified by: Kim Sungjin                                                  */
/*---------------------------------------------------------------------------*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include "common.h"
#include "skvslib.h"
#include <fcntl.h> // added
/*---------------------------------------------------------------------------*/
struct thread_args
{
    int listenfd;
    int idx;
    struct skvs_ctx *ctx;

/*---------------------------------------------------------------------------*/
    /* free to use */

/*---------------------------------------------------------------------------*/
};
/*---------------------------------------------------------------------------*/
volatile static sig_atomic_t g_shutdown = 0;
/*---------------------------------------------------------------------------*/
void *handle_client(void *arg)
{
    TRACE_PRINT();
    struct thread_args *args = (struct thread_args *)arg;
    struct skvs_ctx *ctx = args->ctx;
    int idx = args->idx;
    int listenfd = args->listenfd;
/*---------------------------------------------------------------------------*/
    /* free to declare any variables */
    int clientfd, res, len, off;
    char buf[BUFFER_SIZE + 1];

/*---------------------------------------------------------------------------*/

    free(args);
    printf("%dth worker ready\n", idx);

/*---------------------------------------------------------------------------*/
    /* edit here */
    while (!g_shutdown) {
        clientfd = accept(listenfd, NULL, NULL);

        if (clientfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("accept");
                break;
            }
        }
        int flags = fcntl(clientfd, F_GETFL, 0);
        fcntl(clientfd, F_SETFL, flags | O_NONBLOCK);

        while (!g_shutdown) {
            res = read(clientfd, buf, sizeof(buf) - 1);

            if (res < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    usleep(1000);
                    continue;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    perror("read");
                    break;
                }
            } else if (res == 0) {
                printf("Connection closed by client\n");
                break;
            }

            const char *resp = skvs_serve(ctx, buf, res);
            char *result = strdup(resp);
            strcat(result, "\n");

            len = strlen(result);
            off = 0;

            while (off < len) {
                res = write(clientfd, result + off, len - off);
                if (res <= 0) {
                    perror("write");
                    break;
                }
                off += res;
            }
        }
        close(clientfd);

    }
/*---------------------------------------------------------------------------*/

    return NULL;
}
/*---------------------------------------------------------------------------*/
/* Signal handler for SIGINT */
void handle_sigint(int sig)
{
    TRACE_PRINT();
    printf("\nReceived SIGINT, initiating shutdown...\n");
    g_shutdown = 1;
}
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    size_t hash_size = DEFAULT_HASH_SIZE;
    char *ip = DEFAULT_ANY_IP;
    int port = DEFAULT_PORT, opt;
    int num_threads = NUM_THREADS;
    int delay = RWLOCK_DELAY;
/*---------------------------------------------------------------------------*/
    /* free to declare any variables */
    int s;

    
/*---------------------------------------------------------------------------*/

    /* parse command line options */
    while ((opt = getopt(argc, argv, "p:t:s:d:h")) != -1)
    {
        switch (opt)
        {
        case 'p':
            port = atoi(optarg);
            break;
        case 't':
            num_threads = atoi(optarg);
            break;
        case 's':
            hash_size = atoi(optarg);
            if (hash_size <= 0)
            {
                perror("Invalid hash size");
                exit(EXIT_FAILURE);
            }
            break;
        case 'd':
            delay = atoi(optarg);
            break;
        case 'h':
        default:
            printf("Usage: %s [-p port (%d)] "
                   "[-t num_threads (%d)] "
                   "[-d rwlock_delay (%d)] "
                   "[-s hash_size (%d)]\n",
                   argv[0],
                   DEFAULT_PORT,
                   NUM_THREADS,
                   RWLOCK_DELAY,
                   DEFAULT_HASH_SIZE);
            exit(EXIT_FAILURE);
        }
    }

/*---------------------------------------------------------------------------*/
    /* edit here */
    signal(SIGINT, handle_sigint);

    struct sockaddr_in saddr;

    if ((s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) < 0)  {
        perror("socket()");
        return -1;     
    }

    int optval = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(s);
        return -1;
    }
    
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(ip);
    saddr.sin_port = htons(port);
    if (bind(s, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("bind()");
        close(s);
        return -1;     
    }

    if (listen(s, num_threads) < 0) {
        perror("listen()");
        close(s);
        return -1;     
    }
    printf("Server listening on %s:%d\n", ip, port);

    struct skvs_ctx *ctx = skvs_init(hash_size, delay);
    if (!ctx) {
        perror("SKVS initialization failed");
        close(s);
        exit(EXIT_FAILURE);
    }

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->listenfd = s;
        args->idx = i;
        args->ctx = ctx;

        pthread_create(&threads[i], NULL, handle_client, args);
    }

    while (!g_shutdown) {
        sleep(1);
    }

    printf("Shutting down server...\n");

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    skvs_destroy(ctx, 1);
    close(s);
/*---------------------------------------------------------------------------*/

    return 0;
}
