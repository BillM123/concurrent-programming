#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "mysem.h"

#define MAX_CARS 6
#define TIME_TO_CROSS 1

enum {BLUE, RED};

typedef struct car{
    mysem_t *mySemGroup;
    mysem_t *otherSemGroup;
    mysem_t *bridgemtx;
    mysem_t *bridgeSem;
    mysem_t *exitmtx;
    int *bridgeEmptySpace;
    int bridgeCapacity;
    int *carsToPass;
    int *carsReachedBridge;
    int color;
}car_t;

void *car(void *args){
    car_t *arg = (car_t*)args;

    printf("New car waiting\n");

    if(mysem_down(arg->mySemGroup) == -1){free(arg); return NULL;} 
    if(mysem_down(arg->exitmtx) == -1){free(arg); return NULL;}

    printf("New car heading to bridge\n");
    if(arg->color == BLUE){
        printf("New car is blue\n");
    }
    else {
        printf("New car is red\n");
    }

    if(*(arg->carsToPass) < MAX_CARS-1){
        (*(arg->carsToPass))++;
        if(mysem_up(arg->mySemGroup) == -1){free(arg); return NULL;};
    }
    if(mysem_up(arg->exitmtx) == -1){free(arg); return NULL;};

    printf("Car waiting pre bridge\n");

    //HERE: This should be executed before turn change
    if(mysem_down(arg->bridgeSem) == -1){free(arg); return NULL;} //bridge
    if(mysem_down(arg->bridgemtx) == -1){free(arg); return NULL;}
    (*(arg->carsReachedBridge))++;
    (*(arg->bridgeEmptySpace))--;
    printf("Entered bridge: Reached: %d Remaining space: %d\n", *(arg->carsReachedBridge), *(arg->bridgeEmptySpace));
    if (*(arg->bridgeEmptySpace) > 0) { if(mysem_up(arg->bridgeSem) == -1){free(arg); return NULL;}; }
    if(mysem_up(arg->bridgemtx) == -1){free(arg); return NULL;};

    printf("Car entering bridge\n");
    sleep(TIME_TO_CROSS); //cross the bridge
    
    
    if(mysem_down(arg->exitmtx) == -1){free(arg); return NULL;}
    printf("Car exiting bridge\n");

    if(mysem_down(arg->bridgemtx) == -1){free(arg); return NULL;}
    (*(arg->bridgeEmptySpace))++;
    if (*(arg->bridgeEmptySpace) == 1) { if(mysem_up(arg->bridgeSem) == -1){free(arg); return NULL;}; }
    printf("Exited bridge: Reached: %d Remaining space: %d\n", *(arg->carsReachedBridge), *(arg->bridgeEmptySpace));

    if(*(arg->carsReachedBridge) == MAX_CARS && *(arg->bridgeEmptySpace) == arg->bridgeCapacity){
        *(arg->carsToPass) = 0;
        *(arg->carsReachedBridge) = 0;

        printf("\n\n\nFilpping traffic...\n\n\n");
        if(mysem_up(arg->otherSemGroup) == -1){free(arg); return NULL;};
    }

    if(mysem_up(arg->bridgemtx) == -1){free(arg); return NULL;};
    if(mysem_up(arg->exitmtx) == -1){free(arg); return NULL;};

    free(arg);
    return NULL;
}

int main(int argc, char **argv){
    if(argv[1] == NULL){
        printf("First argument should be bridge capacity\n");
        return -1;
    }
    int bridgeCapacity = atoi(argv[1]);
    int newCarNum;
    int newCarColour;

    int bridgeEmptySpace = bridgeCapacity;
    int carsToPass = 0;
    int carsReachedBridge = 0;

    pthread_t carThread;

    mysem_t blueCarsSem = {0,0,0};
    mysem_t redCarsSem = {0,0,0};
    mysem_t bridgeSem = {0,0,0};
    mysem_t bridgemtx = {0,0,0};
    mysem_t exitmtx = {0,0,0};

    mysem_init(&blueCarsSem, 1);

    mysem_init(&redCarsSem, 0);

    mysem_init(&bridgeSem, 1);

    mysem_init(&bridgemtx, 1);

    mysem_init(&exitmtx, 1);

    printf("Enter: |car colour| |num of new cars|\n");
    printf("Note: 0 for blue 1 for red, blue cars have first turn\n");
    while (scanf("%d %d", &newCarColour, &newCarNum) != EOF){
        for(int i = 0; i < newCarNum; i++){
            car_t *tmp = malloc(sizeof(car_t));

            if(newCarColour == BLUE){
                tmp->mySemGroup = &blueCarsSem;
                tmp->otherSemGroup = &redCarsSem;
                tmp->color = BLUE;
            }
            else{
                tmp->mySemGroup = &redCarsSem;
                tmp->otherSemGroup = &blueCarsSem;
                tmp->color = RED;
            }
            tmp->bridgeSem = &bridgeSem;
            tmp->bridgemtx = &bridgemtx;
            tmp->exitmtx = &exitmtx;


            tmp->bridgeCapacity = bridgeCapacity;
            tmp->bridgeEmptySpace = &bridgeEmptySpace;
            tmp->carsToPass = &carsToPass;
            tmp->carsReachedBridge = &carsReachedBridge;

            pthread_create(&carThread, NULL, car, tmp);
        }
    }

    //Throw a nuclear bomb at the bridge
    mysem_destroy(&bridgemtx);
    mysem_destroy(&bridgeSem);
    mysem_destroy(&exitmtx);
    if(newCarColour == BLUE){
        mysem_destroy(&redCarsSem);
        mysem_destroy(&blueCarsSem);
    }
    if(newCarColour == RED){
        mysem_destroy(&blueCarsSem);
        mysem_destroy(&redCarsSem);
    }
    pthread_join(carThread, NULL);

    return 0;
}