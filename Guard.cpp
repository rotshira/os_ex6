#include <pthread.h>
#include <main1.cpp>
#include <active_object.cpp>
#include <iostream>
#include <pthread.h>
#include <bits/stdc++.h>
using namespace std;
void* global;
pthread_mutex_t lock_new = PTHREAD_MUTEX_INITIALIZER;
bool can =true;
void guard(void* input){ 
    pthread_mutex_lock(&lock_new);
    if(can){
        can = false;
        global = input;
    }
    can =true;
    pthread_mutex_unlock(&lock_new);
}