/*---------------------------------------------------------------------------*/
/* client.c                                                                  */
/* Author: Junghan Yoon, KyoungSoo Park                                      */
/* Modified by: Kim Sungjin                                                  */
/*---------------------------------------------------------------------------*/
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>
#include <errno.h>
#include "common.h"
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char *ip = DEFAULT_LOOPBACK_IP;
    int port = DEFAULT_PORT;
    int interactive = 0; /* Default is non-interactive mode */
    int opt;

/*---------------------------------------------------------------------------*/
    /* free to declare any variables */
    int s, res, len, off;
    char buffer[BUFFER_SIZE + 1], port_str[6];
/*---------------------------------------------------------------------------*/

    /* parse command line options */
    while ((opt = getopt(argc, argv, "i:p:th")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            if (port <= 1024 || port >= 65536)
            {
                perror("Invalid port number");
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            interactive = 1;
            break;
        case 'h':
        default:
            printf("Usage: %s [-i server_ip_or_domain (%s)] "
                   "[-p port (%d)] [-t]\n",
                   argv[0],
                   DEFAULT_LOOPBACK_IP, 
                   DEFAULT_PORT);
            exit(EXIT_FAILURE);
        }
    }

/*---------------------------------------------------------------------------*/
    /* edit here */
    /* create a socket for connection */
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        return -1;     
    }

    struct addrinfo hints, *ai, *ai_it;

    /* domain name lookup */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_str, sizeof(port_str), "%d", port);
    res = getaddrinfo(ip, port_str, &hints, &ai);
    if (res != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(res));
        exit(-1);
    }
    
    /* try connecting to next address if any fails */
    ai_it = ai; 
    while (ai_it != NULL) {        
        if (connect(s, ai_it->ai_addr, ai_it->ai_addrlen) == 0)
        break;      
        ai_it = ai_it->ai_next;
    }
    freeaddrinfo(ai);
    if(ai_it == NULL){
        fprintf(stderr, "Could not connect to %s:%d\n", ip, port);
        exit(EXIT_FAILURE);
    }

    if(interactive){
        printf("Connected to %s:%d\n", ip, port);
        while (1){
            printf("Enter command: ");
            fgets(buffer, sizeof(buffer), stdin);
            if(strcmp(buffer, "\n") == 0) break;
            
            len = strlen(buffer);
            off = 0;
            while (off < len) {
            if ((res = write(s, buffer + off, len - off)) <= 0) {
                perror("write");
                exit(-1);
            }
            off += res;
            }
            if ((res = read(s, buffer, sizeof(buffer)-1)) <= 0) {
                if (res == 0) printf("Server reply: Connection closed by server\n");
                else perror("read");
                break;
            }
            buffer[res] = 0;
            printf("Server reply: %s", buffer);
        }
    }
    else{
        while (fgets(buffer, sizeof(buffer), stdin) != NULL && strcmp(buffer, "\n") != 0){
            len = strlen(buffer);
            off = 0;
            while (off < len) {
                if ((res = write(s, buffer + off, len - off)) <= 0) {
                    perror("write");
                    exit(-1);
                }
                off += res;
            }
            if ((res = read(s, buffer, sizeof(buffer)-1)) <= 0) {
                printf("READ ERROR\n");
                perror("read");
                break;
            }
            buffer[res] = 0;
            printf("%s", buffer);
        }
    }

    /* Cleanup */
    close(s);
/*---------------------------------------------------------------------------*/

    return 0;
}
