#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define PORT "3490"
#define BACKLOG 5

void sigchld_handler(int socket);
void* get_in_addr(struct sockaddr* sa);

int main(void)
{
    int status, sock, con_sock;
    struct addrinfo hints,* servinfo,* p;
    struct sockaddr_storage client_addr;
    socklen_t client_addrsize;
    int yes = 1;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);    // empty struct
    hints.ai_family = AF_UNSPEC;        // IPv4 or v6
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;        // wild card the ip

    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
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

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("server: waiting for connections...\n");

    while (1) {
        client_addrsize = sizeof client_addr;
        if ((con_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addrsize)) == -1){
            perror("accept");
            continue;
        }
       
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) {
            close(sock);
            if (send(con_sock, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(con_sock);
            exit(EXIT_SUCCESS);
        }

        close(con_sock);
    }

    return 0;
}

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}