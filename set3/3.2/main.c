#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct slave{
    long num;
    int id;
    int *whoAmI;
    pthread_mutex_t *mtx;
    pthread_cond_t *workerQueue;
    pthread_cond_t *AvailableQueue;
    pthread_cond_t *ReadyQueue;
    pthread_cond_t *StartingCond;
    int *workersAvailable;
}worker_args;

void *worker(void *thread_arg);

int main(int argc, char *argv[]){
    if(argv[1] == NULL || atoi(argv[1]) <= 0){
        printf("Number of workers should be first argument\n");
        return -1;
    }

    //initialise variables
    long inputValue = 0;
    int whoAmI = 0;
    int number_workers = atoi(argv[1]);

    //create table for proccesses and arguments
    pthread_t *threadTable = (pthread_t*)malloc(atoi(argv[1])*sizeof(pthread_t)); 
    worker_args *workerArgs = (worker_args*)malloc(atoi(argv[1])*sizeof(worker_args));

    //create condition variable queues and the mutex
    pthread_mutex_t *mtx         = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *workerQueue  = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *AvailableQueue = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *ReadyQueue = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *StartingCond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
   
    //Initialise mutex and condition variables
    pthread_mutex_init(mtx, NULL);
    pthread_cond_init(workerQueue, NULL);
    pthread_cond_init(AvailableQueue, NULL);
    pthread_cond_init(ReadyQueue, NULL);
    pthread_cond_init(StartingCond, NULL);

    //create all workers and create them
    for(int i = 0; i < atoi(argv[1]); i++){
        
        workerArgs[i].num = 0;
        workerArgs[i].id = i;
        workerArgs[i].whoAmI = &whoAmI;
        workerArgs[i].mtx = mtx;
        workerArgs[i].workerQueue = workerQueue;
        workerArgs[i].AvailableQueue = AvailableQueue;
        workerArgs[i].ReadyQueue = ReadyQueue;
        workerArgs[i].workersAvailable = &number_workers;
        workerArgs[i].StartingCond = StartingCond;

        pthread_create(&threadTable[i], NULL, worker, &workerArgs[i]);
    }

    //feed workers input while input is given
    while(scanf("%ld", &inputValue) != EOF){
        printf("main: locking mutex\n");
        pthread_mutex_lock(mtx);
        printf("Checking if there are any free workers %d\n", number_workers);
        if(number_workers == 0){
            printf("No free workers found\nwaiting on AvailableQueue\n");
            pthread_cond_wait(AvailableQueue, mtx);
        }
        printf("main: waking worker\n");
        pthread_cond_signal(workerQueue);
        printf("Waiting for Workers info...\n");
        pthread_cond_wait(ReadyQueue, mtx);
        printf("main: Giving worker %d num %ld\n", whoAmI, inputValue);
        workerArgs[whoAmI].num = inputValue;
        number_workers--;
        printf("signaling worker\n");
        pthread_cond_signal(StartingCond);
        printf("main: unlocking mutex\n");
        pthread_mutex_unlock(mtx);
    }

    //order proccesses to terminate
    fprintf(stderr, "Terminating...\n");
    
    pthread_cond_destroy(workerQueue);
    pthread_cond_destroy(AvailableQueue);
    pthread_cond_destroy(ReadyQueue);
    pthread_mutex_destroy(mtx);

    for (int i = 0; i < atoi(argv[1]); i++) {
        pthread_join(threadTable[i], NULL);
    }

    free(mtx);
    free(ReadyQueue);
    free(workerQueue);
    free(AvailableQueue);
    free(threadTable);
    free(workerArgs);

    return 0;
}

void *worker(void *thread_arg){
    long cnt = 0;
    worker_args *arg = (worker_args*)thread_arg;

    while (1) {
        printf("Worker %d: locking mutex\n", arg->id);
        if(pthread_mutex_lock(arg->mtx) == -1) break;
        printf("Worker %d: waiting in queue\n", arg->id);
        if(pthread_cond_wait(arg->workerQueue, arg->mtx) == -1) break;
        printf("Worker %d: giving main my id\n", arg->id);
        *(arg->whoAmI) = arg->id;
        printf("Worker %d: signaling main\n", arg->id);
        if(pthread_cond_signal(arg->ReadyQueue) == -1) break;
        printf("Worker %d: waiting for a signal from main\n", arg->id);
        if(pthread_cond_wait(arg->StartingCond, arg->mtx) == -1) break;
        printf("Worker %d: Unlocking mutex\n", arg->id);
        if(pthread_mutex_unlock(arg->mtx) == -1) break;
        printf("Worker %d: Starting work on %lu\n", arg->id, arg->num);

        // Code concerning primality testing originates from geeksforgeeks:
        // https://www.geeksforgeeks.org/c/c-program-to-check-whether-a-number-is-prime-or-not/
        if (arg->num <= 1)
            printf("\n\nWorker %d: %ld is NOT prime\n\n",arg->id, arg->num);
        else {

            // Check for divisors from 1 to n
            for (long i = 1; i <= arg->num; i++) {

                // Check how many number is divisible
                // by n
                if (arg->num % i == 0)
                    cnt++;
                if(cnt > 2) break;
            }

            // If n is divisible by more than 2 numbers
            // then it is not prime
            if (cnt > 2)
                printf("\nWorker %d: %ld is NOT prime\n\n", arg->id, arg->num);

            // else it is prime
            else
                printf("\nWorker %d: %ld is prime\n\n", arg->id, arg->num);
        }
        
        cnt = 0;

        printf("Worker %d: locking mutex\n", arg->id);
        if(pthread_mutex_lock(arg->mtx) == -1) break;
        //if all workers are working main is probably asleep
        printf("Worker %d: checking if i should signal main\n", arg->id);
        if(arg->workersAvailable == 0){
            if(pthread_cond_signal(arg->AvailableQueue) == -1) break;
        }
        printf("Worker %d: increasing free workers = %d\n", arg->id, *(arg->workersAvailable));
        (*(arg->workersAvailable))++;
        printf("Worker %d: Unlocking mutex\n", arg->id);
        if(pthread_mutex_unlock(arg->mtx) == -1) break;
        printf("Worker %d: incresed free workers to = %d\n", arg->id, *(arg->workersAvailable));
    }
    return NULL;
}
