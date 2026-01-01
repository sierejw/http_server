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
#include "http.h"
#include "server.h"

#define BUFFSIZ 1024

void sigchld_handler(int socket);
void* get_in_addr(struct sockaddr* sa);

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: main portnum\n");
        exit(EXIT_FAILURE);
    }

    int sock, con_sock;
    struct sockaddr_storage client_addr;
    socklen_t client_addrsize;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    char buf[BUFFSIZ];
    struct response resp;
    int resp_len;

    sock = setup_server(argv[1]);

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
            if (recv(con_sock, buf, BUFFSIZ, 0) == -1)
                perror("recv");
            construct_response(&resp, buf, resp_len);
            char response_buf[100] = {0};
            snprintf(response_buf, sizeof(response_buf), "HTTP/%.1f %d %s\r\n\r\n%s", resp.ver, resp.status, resp.reason, resp.message);
            printf("%s", response_buf);
            char* p = response_buf;
            int size = 0;
            /*if (strchr(p, '\0')) {
                printf("yes\n");
            }*/
            while (*p != '\0'){
                size++;
                p++;
            }
            if (send(con_sock, response_buf, size, 0) == -1)
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