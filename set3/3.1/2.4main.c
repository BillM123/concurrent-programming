#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "mysem.h"

typedef struct parg{
    mysem_t *passengerSem;
    mysem_t *trainSem;
    mysem_t *waitingQueue;
    int capacity;
    int *numOfPassengers;
    int *term;
}passengerArg;

typedef struct targ{
    mysem_t *passengerSem;
    mysem_t *trainSem;
    int *term;
}trainArg;

void *train(void *args){
    trainArg *arg = (trainArg*)args;

    while(1){
        printf("(Train)Sleeping...\n");
        mysem_down(arg->trainSem);
        //onboarding

        //travel
        printf("(Train)Travelling...\n");
        sleep(5);
        printf("(Train)Finished travelling!\n");

        //offboarding
        //assume start of path = end of path
        mysem_up(arg->passengerSem);
        if(*arg->term == 1){
            mysem_down(arg->passengerSem);
            return NULL;   
        }
    }

    return NULL;
}

void *passenger(void *args){
    passengerArg *arg = (passengerArg*)args;
    int debugNum = rand();
    
    //onboarding
    printf("(%d)(Onboarding)Waiting...\n", debugNum);
    mysem_down(arg->waitingQueue);
    printf("(%d)Onboarding...\n", debugNum);

    (*(arg->numOfPassengers))++;
    printf("(%d) Num Of Passengers: %d\n", debugNum, *arg->numOfPassengers);
    if(*(arg->numOfPassengers) == arg->capacity){
        printf("(%d) Waking train\n", debugNum);
        mysem_up(arg->trainSem);
    }
    else{
        mysem_up(arg->waitingQueue);
    }

    //travel
    mysem_down(arg->passengerSem);

    //offboarding
    printf("(%d)Travelled, offboarding...\n", debugNum);
    (*(arg->numOfPassengers))--;
    if(*(arg->numOfPassengers) == 0){
        if(*arg->term == 1){
            mysem_up(arg->passengerSem);
            free(arg);
            return NULL;    
        }
        printf("(%d)Waking new queue\n", debugNum);
        mysem_up(arg->waitingQueue);
    }
    else {
        mysem_up(arg->passengerSem);
    }
    free(arg);
    return NULL;
}

int main(int argc, char **argv){
    if(argv[1] == NULL){
        printf("First argument should be train capacity\n");
        return -1;
    }
    int trainCapacity = atoi(argv[1]);
    int newPassengerNum;
    int numOfPassengers = 0;
    int term = 0;

    mysem_t trainSem = {0, 0, 0};
    mysem_t passengerSem = {0, 0, 0};
    mysem_t waitingQueue = {0, 0, 0};

    mysem_init(&trainSem, 0);
    printf("Train semid is %d\n", trainSem.semid);
    
    mysem_init(&passengerSem, 0);
    printf("Passenger semid is %d\n", passengerSem.semid);
    
    mysem_init(&waitingQueue, 1);
    printf("Queue semid is %d\n", waitingQueue.semid);
    

    pthread_t trainThread;
    trainArg *trainStruct = malloc(sizeof(trainArg));
    trainStruct->trainSem = &trainSem;
    trainStruct->passengerSem = &passengerSem;
    trainStruct->term = &term;

    pthread_create(&trainThread, NULL, train, trainStruct);


    printf("Enter number of new passengers\n");
    while (scanf("%d", &newPassengerNum) != EOF) {
        for(int i = 0; i < newPassengerNum; i++){
            passengerArg *tmp = malloc(sizeof(passengerArg));

            tmp->numOfPassengers = &numOfPassengers;
            tmp->capacity = trainCapacity;
            tmp->trainSem = &trainSem;
            tmp->passengerSem = &passengerSem;
            tmp->waitingQueue = &waitingQueue;
            tmp->term = &term;

            pthread_t passengerThread; //need something like this to prevent segfault
            
            pthread_create(&passengerThread, NULL, passenger, tmp);
        }        

    }
    term = 1;
    mysem_up(&trainSem); //theoretically counts as a middleman, 
                           //but both train and passengers should be waiting
                           //and train has to be notified of termination somehow 
    pthread_join(trainThread, NULL);
    free(trainStruct);

    mysem_destroy(&trainSem);
    mysem_destroy(&passengerSem);
    mysem_destroy(&waitingQueue);

    return 0;
}