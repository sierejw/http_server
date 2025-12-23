#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "server.h"

#define BACKLOG 5

int setup_server(const char* port) {
    int status, sock;
    struct addrinfo hints,* servinfo,* p;
    int yes = 1;
    
    memset(&hints, 0, sizeof hints);    // empty struct
    hints.ai_family = AF_UNSPEC;        // IPv4 or v6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;        // wild card the ip

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

       if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
       } 

       if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            close(sock);
            perror("server: bind");
            continue;
       }

       break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(EXIT_FAILURE);
    }
   
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return sock;
}