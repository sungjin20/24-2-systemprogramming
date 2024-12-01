/*---------------------------------------------------------------------------*/
/* server.c                                                                  */
/* Author: Junghan Yoon, KyoungSoo Park                                      */
/* Modified by: (Your Name)                                                  */
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
    

/*---------------------------------------------------------------------------*/

    free(args);
    printf("%dth worker ready\n", idx);

/*---------------------------------------------------------------------------*/
    /* edit here */
    







    
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
    







    
/*---------------------------------------------------------------------------*/

    return 0;
}
