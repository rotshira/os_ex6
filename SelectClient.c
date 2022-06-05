
/*
** pollserver.c -- a cheezy multiperson chat server
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>

#define PORT "3491" // Port we're listening on
#define MAXDATASIZE 50 //  max number of bytes we can get at once
int _connect ,sockfd;

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
      return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// fun to cliente receive
void *_receive(void *arg)
{
    int length =0;
    char msg[1024] = {0};
    while ((length = recv(sockfd, msg, 1024, 0)) != -1){
        if (!length){
            _connect = 0;
            break;
        }
        printf("\nreceive: %s\n",msg);
        bzero(msg, 1024);
    }
    return NULL;
}

// fun to se receive
void *_send(void *arg){
    char msg[1024] = {0};
    while (_connect != 0)
    {    
        printf("send: ");
        gets(msg);
        if (send(sockfd, msg, strlen(msg) + 1, 0) == -1)
        {
            _connect = 0;
            perror("send");
        }
        bzero(msg, 1024);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    char buf[MAXDATASIZE]; 
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "The connection failed!!\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    printf("connect - %s\n", s);
    freeaddrinfo(servinfo);

    pthread_t thread_1,thread_2;
    _connect = 1;
    pthread_create(&thread_1, NULL, _receive, NULL);
    pthread_create(&thread_2, NULL, _send, NULL);
    pthread_join(thread_1, NULL);
    pthread_join(thread_1, NULL);
    pthread_kill(thread_2, 0);
    pthread_kill(thread_2, 0);
    close(sockfd);
    return 0;
}