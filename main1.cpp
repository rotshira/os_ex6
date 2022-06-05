#include <pthread.h>
#include <iostream>
#include <bits/stdc++.h>
using namespace std;
// took from https://www.geeksforgeeks.org/queue-linked-list-implementation/
pthread_mutex_t lock_new = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


// A C program to demonstrate linked list based implementation of queue
#include <stdio.h>
#include <stdlib.h>

// A linked list (LL) node to store a queue entry
struct QNode {
	void* key;
	struct QNode* next;
};

// The queue, front stores the front node of LL and rear stores the
// last node of LL
struct Queue {
	struct QNode *front, *rear;
};

// A utility function to create a new linked list node.
struct QNode* newNode(void* k)
{
	struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
	temp->key = k;
	temp->next = NULL;
	return temp;
}

// A utility function to create an empty queue
struct Queue* createQ()
{
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
	q->front = q->rear = NULL;
	return q;
}

// The function to add a key k to q
void enQ(struct Queue* q, void* k)
{
	// Create a new LL node
	pthread_mutex_lock(&lock_new);
	struct QNode* temp = newNode(k);

	// If queue is empty, then new node is front and rear both
	if (q->rear == NULL) {
		q->front = q->rear = temp;
		return;
	}

	// Add the new node at the end of queue and change rear
	q->rear->next = temp;
	q->rear = temp;
	pthread_mutex_unlock(&lock_new);
	pthread_cond_signal(&cond);
}

// Function to remove a key from given queue q
void deQ(struct Queue* q)
{
	 pthread_mutex_lock(&lock_new);
	// If queue is empty, return NULL.
	if (q->front == NULL)
		return;

	// Store previous front and move front one node ahead
	struct QNode* temp = q->front;

	q->front = q->front->next;

	// If front becomes NULL, then change rear also as NULL
	if (q->front == NULL)
		q->rear = NULL;

	free(temp);
	pthread_cond_wait(&cond,&lock_new);
    pthread_mutex_unlock(&lock_new);
}
void destroyQ(struct Queue* q)
    {
        pthread_mutex_unlock(&lock_new);
        while(q->front != q->rear){
            deQ(q);
        }
        free(q->front);
        pthread_mutex_unlock(&lock_new);
    }

// Driver Program to test above functions
// int main()
// {
// 	// struct Queue* q = createQ();
// 	// enQ(q, 10);
// 	// enQ(q, 20);
// 	// deQ(q);
// 	// deQ(q);
// 	// enQ(q, 30);
// 	// enQ(q, 40);
// 	// enQ(q, 50);
// 	// deQ(q);
// 	// printf("Queue Front : %d \n", q->front->key);
// 	// printf("Queue Rear : %d", q->rear->key);
// 	// return 0;
// }


