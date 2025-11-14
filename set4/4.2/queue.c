#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "include/queue.h"

// Function to create a new queue
Queue *createQueue() {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    if (!queue) {
        perror("Failed to create queue");
        exit(EXIT_FAILURE);
    }
    queue->head = queue->tail = NULL;
    queue->size = 0;
    return queue;
}

// Function to create a new node
Node *createNode(void *data) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) {
        perror("Failed to create node");
        exit(EXIT_FAILURE);
    }
    node->data = data;
    node->next = NULL;
    return node;
}

// Function to check if the queue is empty
int isEmpty(Queue *queue) {
    return queue->size == 0;
}

// Function to enqueue an element
void enqueue(Queue *queue, void *data) {
    Node *newNode = createNode(data);
    if (isEmpty(queue)) {
        queue->head = queue->tail = newNode;
    } else {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    queue->size++;
}

// Function to dequeue an element
void *dequeue(Queue *queue) {
    if (isEmpty(queue)) {
        fprintf(stderr, "Queue underflow\n");
        return NULL;
    }
    Node *temp = queue->head;
    void *data = temp->data;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    free(temp);
    queue->size--;
    return data;
}

// Function to peek at the head element without dequeuing
void *peek(Queue *queue) {
    if (isEmpty(queue)) {
        fprintf(stderr, "Queue is empty\n");
        return NULL;
    }
    return queue->head->data;
}

// Function to free the queue
void freeQueue(Queue *queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}
