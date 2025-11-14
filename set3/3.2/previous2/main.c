#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "mysem.h"

typedef struct slave{
    long num;
    int id;
    int *whoAmI;
    mysem_t *workerQueue;
    mysem_t *mtx1;
    mysem_t *mtx2;
}worker_args;

void *worker(void *thread_arg);

int main(int argc, char *argv[]){
    if(argv[1] == NULL || atoi(argv[1]) <= 0){
        printf("Number of workers should be first argument\n");
        return -1;
    }

    //initialize variables
    long inputValue = 0;
    int whoAmI = 0;
    
    //create table for proccesses and arguments
    pthread_t *threadTable = (pthread_t*)malloc(atoi(argv[1])*sizeof(pthread_t)); 
    worker_args *workerArgs = (worker_args*)malloc(atoi(argv[1])*sizeof(worker_args));

    //*init semaphores*/
    mysem_t workerQueue = {0,0,0};
    mysem_init(&workerQueue, 0);

    mysem_t mtx1 = {0,0,0};
    mysem_init(&mtx1, 0);

    mysem_t mtx2 = {0,0,0};
    mysem_init(&mtx2, 0);
    
    //create all workers
    for(int i = 0; i < atoi(argv[1]); i++){
        
        workerArgs[i].num = 0;
        workerArgs[i].id = i;
        workerArgs[i].whoAmI = &whoAmI;
        workerArgs[i].workerQueue = &workerQueue;
        workerArgs[i].mtx1 = &mtx1;
        workerArgs[i].mtx2 = &mtx2;

        pthread_create(&threadTable[i], NULL, worker, &workerArgs[i]);
    }

    //feed workers input while input is given
    while(scanf("%ld", &inputValue) != EOF){
        printf("main: waking worker\n");
        mysem_up(&workerQueue);
        printf("main: downing mtx1\n");
        mysem_down(&mtx1);
        printf("main: Giving worker %d num %ld\n", whoAmI, inputValue);
        workerArgs[whoAmI].num = inputValue;
        printf("main: upping mtx2\n");
        mysem_up(&mtx2);
    }

    //order proccesses to terminate
    fprintf(stderr, "Terminating...\n");
    
    mysem_destroy(&workerQueue);
    mysem_destroy(&mtx1);
    mysem_destroy(&mtx2);
    

    //Destroy the master thread
    mysem_destroy(workerArgs->mtx1);

    for (int i = 0; i < atoi(argv[1]); i++) {
        pthread_join(threadTable[i], NULL);
    }
    free(threadTable);
    free(workerArgs);

    return 0;
}

void *worker(void *thread_arg){
    long cnt = 0;
    worker_args *arg = (worker_args*)thread_arg;

    while (1) {
        printf("Worker %d: waiting in queue\n", arg->id);
        if(mysem_down(arg->workerQueue) == -1)break;
        printf("Worker %d: giving main my id\n", arg->id);
        *(arg->whoAmI) = arg->id;
        printf("Worker %d: Upping mtx1\n", arg->id);
        if(mysem_up(arg->mtx1) == -1)break;
        printf("Worker %d: Downing mtx2\n", arg->id);
        if(mysem_down(arg->mtx2) == -1)break;
        printf("Worker %d: Starting work on %lu\n", arg->id, arg->num);

        // Code concerning primality testing originates from geeksforgeeks:
        // https://www.geeksforgeeks.org/c/c-program-to-check-whether-a-number-is-prime-or-not/
        if (arg->num <= 1)
            printf("%ld is NOT prime\n", arg->num);
        else {

            // Check for divisors from 1 to n
            for (long i = 1; i <= arg->num/2; i++) {

                // Check how many number is divisible
                // by n
                if (arg->num % i == 0)
                    cnt++;
                if(cnt > 2) break;
            }

            // If n is divisible by more than 2 numbers
            // then it is not prime
            if (cnt > 2)
                printf("Worker %d: %ld is NOT prime\n", arg->id, arg->num);

            // else it is prime
            else
                printf("Worker %d: %ld is prime\n", arg->id, arg->num);
        }
        
        cnt = 0;
        
    }
    printf("Worker %d: Error in semaphore call, asumming termination notice\n", arg->id);
    return NULL;
}
