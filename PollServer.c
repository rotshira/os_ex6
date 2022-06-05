/*
** pollserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <poll.h>
#include "Reactor.h"

#define PORT "3491"  // Port we're listening on

int fd_count;
int listener;
struct pollfd *pfds;
char buf[1024];

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

preactor newReactor()
{
    preactor res = (preactor)(malloc(sizeof(reactor)));
    return res;
}

void InstallHandler(preactor rc, pfunc newFunc, int file_des)
{
    rc->function = newFunc;
    rc->f_ID = file_des;
    preqests rq = (preqests)(malloc(sizeof(reqests)));
    rq->f_ID = file_des;
    rq->reac = rc;
    pthread_create(&rc->t_id, NULL, newFunc, rq);
}
void RemoveHandler(preactor reactor_1, int fd_free)
{
    pthread_join(reactor_1->t_id, NULL);
    reactor_1->f_ID = -1;
    reactor_1->function = NULL;
}


int get_listener_socket(void)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0){
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
       listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
            continue;
        

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }
        break;
    }
    freeaddrinfo(ai); // All done with this
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size){
        *fd_size *= 2; // Double it
        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];
    (*fd_count)--;
}
void *data_receve(void *arg)
{
    preqests req = &((preqests)arg)[0];
    int  newFd = req-> f_ID;
    char buf[1024]; // Buffer for client data
    while(1){
        int number = recv(newFd, buf, sizeof(buf), 0);
        if (number <= 0)
        {
            close(newFd);
            return NULL;
        }
        else{
            for (int i = 0; i < fd_count; i++){
                int id_client = pfds[i].fd;
                 if (id_client != listener && id_client != newFd)
                    send(id_client, buf, number, 0);
            }
            bzero(buf, 1024);
        }
    }
}



// Main
int main(void)
{
    int newfd;                          
    struct sockaddr_storage remoteaddr; 
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];

    // Start off with room for 5 connections
    // (We'll realloc as necessary)
    fd_count = 0;
    int fd_size = 5;
    pfds = (struct pollfd *)malloc(sizeof(*pfds) * fd_size);
    // Set up and get a listening socket
   listener = get_listener_socket();
    if (listener == -1){
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Add the listener to set
    pfds[0].fd = listener;
    pfds[0].events = POLLIN; // Report ready to read on incoming connection

    fd_count = 1; // For the listener
    // Main loop
    while(1)
    {
        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1){
            perror("poll");
            exit(1);
        }

        // Run through the existing connections looking for data to read
        for (int i = 0; i < fd_count; i++) {
            // Check if someone's ready to read
            if (pfds[i].revents & POLLIN){ // We got one!!
                if (pfds[i].fd == listener)
                {
                    // If l is ready to read, handle new connection

                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
                        printf("pollserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr *)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd);
                        preactor reactor_1 = (preactor)newReactor();
                        InstallHandler(reactor_1, &data_receve, newfd);
                    }
                }
                else{
                    // If not the listener, we're just a regular client
                    int nbytes = recv(pfds[i].fd, buf, sizeof buf, 0);
                    int sender_fd = pfds[i].fd;

                    if (nbytes <= 0){
                        // Got error or connection closed by client
                        if (nbytes == 0)
                        {
                            // Connection closed
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(pfds[i].fd); // Bye!
                        del_from_pfds(pfds, i, &fd_count);
                    }
                    else
                    {
                        // We got some good data from a client

                        for (int j = 0; j < fd_count; j++)
                        {
                            // Send to everyone!
                            int dest_fd = pfds[j].fd;
                            // Except the l and ourselves
                            if (dest_fd !=listener && dest_fd != sender_fd)
                            {
                                if (send(dest_fd, buf, nbytes, 0) == -1)
                                    perror("send");
                            }
                        }
                    }
                } // END handle data from client
            }     // END got ready-to-read from poll()
        }         // END looping through file descriptors
    }             // END for(;;)--and you thought it would never end!

    return 0;
}