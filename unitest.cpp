#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <string.h>
#include "main1.c"
#include <stdbool.h>
using namespace std;

struct Queue queue_1;
int main()
 {
    struct Queue* q = createQ();
    char* test1 = "shira";
    char* test2 = "amit";
    char* test3 = "os";
    enQ(&queue_1, test1);
    enQ(&queue_1, test2); 
    enQ(&queue_1, test3);
    deQ(&queue_1);
    deQ(&queue_1);
    // printf("Queue Front : %d \n", &queue_1->front->key);
    destroyQ(&queue_1);
     return 0;
 }




