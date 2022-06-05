/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include "active_object.cpp"
// #include "main1.cpp"
#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
 Queue* q1;
 Queue* q2;
 Queue* q3;
 activeobject* one;
 activeobject* two;
 activeobject* three;
struct details{
    char text[1024];
    int new_fd;
};
struct details* det = (details*)malloc(sizeof(details));
void* ceazer_func(void* help)
{
    for(int i=0;i<strlen(det->text);i++)
    {
        if(det->text[i] == 'z'){
            det->text[i] = 'a';
        }
        if(det->text[i] == 'Z'){
            det->text[i]= 'A';
        }
        else{
            det->text[i]=det->text[i]+1;
        }
        
    }
    return det;
}


void* big_letters(void* help){
    for(int i = 0; i<strlen(det->text);i++){
        if( 97 <= det->text[i] && det->text[i] <= 122){
            det->text[i] -= 32;
        }
        if(65 <= det->text[i] && det->text[i] <= 90){
            det->text[i] += 32;
        }  
    }
    return det;
}

void* answer(void* help){
    struct details* det1 = (details*)help;
    if(send(det1->new_fd,det1->text,1024,0)==-1){
      perror("cant send...");
    }
    return NULL;
}

void* client_hendle(void* input){
    // pthread_mutex_lock(&lock_new);
    // cout<<(char*)input<<endl;
    enQ(q1,det->text);
    // pthread_mutex_unlock(&lock_new);
    return NULL;
}
void* fun1(void* help){
    enQ(q2,help);
    return help;
}
void* fun2(void* help){
    enQ(q3,help);
    return help;
}
void* fun3(void* help){
    return help;
}


void sigchld_handler(int s)
{
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

int main(void)
{   
    q1 = createQ();
    q2 = createQ();
    q3 = createQ();
    one = (activeobject*)newAO(q1,ceazer_func, fun1);
    two = (activeobject*)newAO(q2,big_letters, fun2);
    three = (activeobject*)newAO(q3,fun3, answer);
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    pthread_t tid[100];
    int i = 0;
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        int numbytes;
        char buf[1024];
        if ((numbytes = recv(new_fd, buf, 1024, 0)) == -1) {
        perror("recv");
        exit(1);
        }
        memcpy(det->text,buf,sizeof(buf));
        det->new_fd = new_fd;
        if(pthread_create(&tid[i++], NULL, *client_hendle, buf) != 0 ){
            printf("Failed to create thread\n");
        }
        
    
        // printf("%s\n",buf);

        printf("server: got connection from %s\n", s);
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}