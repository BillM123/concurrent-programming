#include <stdio.h>

typedef struct Node {
    void *data;            // Pointer to hold any type of data
    struct Node *next;     // Pointer to the next node
} Node;

// Queue structure
typedef struct Queue {
    Node *head;           // Pointer to the front node
    Node *tail;            // Pointer to the rear node
    size_t size;      
} Queue;


// Function to initialize the queue
Queue *createQueue();

// Function to enqueue an element
void enqueue(Queue *q, void *element);

// Function to dequeue an element
void *dequeue(Queue *q);

// Function to check if the queue is empty
int isEmpty(Queue *q);

// Function to free the entire queue
void freeQueue(Queue *q);

// Peeks at the first element of the queue
void *peek(Queue *queue);