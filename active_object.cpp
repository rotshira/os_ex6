#include <pthread.h>
#include "main1.cpp"
#include <iostream>
#include <pthread.h>
#include <bits/stdc++.h>
using namespace std;

struct activeobject{
    void* (*func1)(void*);
    Queue* q;
    void* (*func2)(void*);
    pthread_t* p;
};

void* newAO(Queue* que,void* func1(void*), void* func2(void*)){
    activeobject *ao = (activeobject*)malloc(sizeof(activeobject));
    ao->func1 = func1;
    ao->func2 = func2;
    ao->q = que;
    while(1){
        void* temp = ao->q->front;
        deQ(ao->q);
        void* temp1 =ao->func1((char*)temp);
        ao->func2(temp1);
    }
    return ao;
}

void destroyAO(struct activeobject* ao){
    free(ao->q);    
    free(ao);
}