#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TRAVEL_TIME 2

typedef struct parg{
    pthread_cond_t *trainCond;
    pthread_cond_t *passengerCond;
    pthread_cond_t *waitingQueue;
    pthread_mutex_t *mtx;
    int capacity;
    int *numOfPassengers;
    int *term;
    int *offboarding;
}passengerArg;

typedef struct targ{
    pthread_cond_t *trainCond;
    pthread_cond_t *passengerCond;
    pthread_mutex_t *mtx;
    int *term;
}trainArg;

void *train(void *args){
    trainArg *arg = (trainArg*)args;

    pthread_mutex_lock(arg->mtx);

    while (1) {
        //wait for passengers
        printf("(Train)Sleeping...\n");
        pthread_cond_wait(arg->trainCond, arg->mtx);

        //travel
        printf("(Train)Travelling...\n");
        sleep(TRAVEL_TIME);
        printf("(Train)Finished travelling!\n");

        //offboarding
        pthread_cond_signal(arg->passengerCond);
        if(*(arg->term) == 1){
            break;
        }
        
    }
    pthread_mutex_unlock(arg->mtx);
    return NULL;
}

void *passenger(void *args){
    passengerArg *arg = (passengerArg*)args;
    int debugNum = rand();

    //Onboarding
    pthread_mutex_lock(arg->mtx);

    printf("(%d)Onboarding...\n", debugNum);

    //wait if train full or if passengers offboarding
    //signal will only be called once offboarding completed
    //so while isnt needed
    if(*(arg->numOfPassengers) == arg->capacity || *(arg->offboarding) == 1){
        printf("(%d)Train full, waiting...\n", debugNum);
        pthread_cond_wait(arg->waitingQueue, arg->mtx);
    }
    (*(arg->numOfPassengers))++;
    
    printf("(%d)Num Of Passengers: %d\n", debugNum, *arg->numOfPassengers);

    //wake either train or next passenger
    if(*(arg->numOfPassengers) == arg->capacity){
        printf("(%d) Waking train\n", debugNum);
        pthread_cond_signal(arg->trainCond);
    }
    else{
        pthread_cond_signal(arg->waitingQueue);
    }
    //wait in train
    pthread_cond_wait(arg->passengerCond, arg->mtx);
    
    pthread_mutex_unlock(arg->mtx);
    
    //offboarding
    pthread_mutex_lock(arg->mtx);
    //signifies to new passengers that offboarding is ongoing
    *(arg->offboarding) = 1; 
    
    printf("(%d)Travelled, offboarding...\n", debugNum);
    (*(arg->numOfPassengers))--;
    //enters if the last arguments
    if(*(arg->numOfPassengers) == 0){
        //enters if programm terminates
        if(*arg->term == 1){
            pthread_cond_signal(arg->passengerCond);

            pthread_mutex_unlock(arg->mtx);
            free(arg);
            return NULL;    
        }
        //allows new passengers to enter the train
        printf("(%d)Waking new queue\n", debugNum);
        *(arg->offboarding) = 0;
        pthread_cond_signal(arg->waitingQueue);
    }
    else {
        //tells nex passenger to offboard
        pthread_cond_signal(arg->passengerCond);
    }
    pthread_mutex_unlock(arg->mtx);

    free(arg);
    return NULL;

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
    int offboarding = 0;

    pthread_mutex_t *mtx         = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *trainCond     = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *passengerCond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *waitingQueue  = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

    pthread_mutex_init(mtx, NULL);
    pthread_cond_init(trainCond, NULL);
    pthread_cond_init(passengerCond, NULL);
    pthread_cond_init(waitingQueue, NULL);

    pthread_t trainThread;
    trainArg *trainStruct = malloc(sizeof(trainArg));
    trainStruct->trainCond = trainCond;
    trainStruct->passengerCond = passengerCond;
    trainStruct->mtx = mtx;
    trainStruct->term = &term;

    pthread_create(&trainThread, NULL, train, trainStruct);

    printf("Enter number of new passengers\n");
    while (scanf("%d", &newPassengerNum) != EOF) {
        for(int i = 0; i < newPassengerNum; i++){
            passengerArg *tmp = malloc(sizeof(passengerArg));

            tmp->numOfPassengers = &numOfPassengers;
            tmp->capacity = trainCapacity;
            tmp->trainCond = trainCond;
            tmp->passengerCond = passengerCond;
            tmp->waitingQueue = waitingQueue;
            tmp->mtx = mtx;
            tmp->term = &term;
            tmp->offboarding = &offboarding;

            pthread_t passengerThread; //need something like this to prevent segfault
            
            pthread_create(&passengerThread, NULL, passenger, tmp);
        }        
    }
    term = 1;
    pthread_cond_signal(trainCond);
    pthread_join(trainThread, NULL);
    free(trainStruct);

    free(mtx);
    free(trainCond);
    free(passengerCond);
    free(waitingQueue);

    return 0;
}