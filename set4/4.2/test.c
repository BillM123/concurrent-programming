#include <stdio.h>
#include <stdlib.h>
#include "include/mythreads.h"

#define NUM_THREADS 5
mysem_t semaphore;
mysem_t semaphore1;

mythr_t threads[NUM_THREADS];
int j=0;

void thread_function(void* arg) {
    int *id =(int *)arg;
    printf("enter thread %d\n", *id);
    printf("Thread %d: Entering critical section\n", *id);
    for (int i = 0; i < 1000000; i++) {
        // printf("Thread %d: Iteration %d\n", *id, i + 1);
        // mythreads_sleep(1);
        //mythreads_sleep(1); // Sleep for 1 second
        mythreads_sem_down(&semaphore);
        j++;
        mythreads_sem_up(&semaphore);
    }
    // mythreads_sleep(1);
    printf("Thread %d: Exiting critical section\n", *id);
    printf("Terminating...\n");
    return;
}

int main() {
    if (mythreads_init() == -1) {
        fprintf(stderr, "Failed to initialize threading library.\n");
        return EXIT_FAILURE;
    }

    if (mythreads_sem_create(&semaphore, 1) == -1) {
        fprintf(stderr, "Failed to create semaphore.\n");
        return EXIT_FAILURE;
    }
    int *id = malloc(sizeof(int));
    mythreads_yield();
    for (int i = 0; i < NUM_THREADS; i++) {
        
        *id = i + 1;
        printf("Creating thread...\n");
        if (mythreads_create(&threads[i], thread_function, id) == -1) {
            fprintf(stderr, "Failed to create thread %d.\n", i + 1);
            return EXIT_FAILURE;
        }
        
    }
    //printf("1\n");
    // print_thread_queue();
    // mythreads_yield();
    printf("Sleeping...\n");
    mythreads_sleep(2);
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("trying2join %d\n",i);
        mythreads_join(&threads[i]);
    }
    //printf("%d\n", j);
        // mythreads_sleep(1);
    for (int i = 0; i < NUM_THREADS; i++) {
        printf("destroying %d\n",i);
        mythreads_destroy(&threads[i]);
    }
    printf("All threads completed.\n");
    free(id);
    return EXIT_SUCCESS;
}
