#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#define nullptr ((void*)0)
typedef void *(*pfunc)(void *arg);

typedef struct Reactor
{
    int f_ID;
    pthread_t t_id;
    pfunc function;
} reactor, *preactor;

typedef struct Reqests
{
    int f_ID;
    preactor reac;
} reqests, *preqests;


preactor newReactor();
void InstallHandler(preactor rc, pfunc newFunc, int file_des);
void RemoveHandler(preactor reac, int fd_free);
