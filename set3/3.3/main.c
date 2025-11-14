#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CARS 5
#define TIME_TO_CROSS 1

enum {BLUE, RED};

typedef struct car{
    pthread_mutex_t *mtx;
    pthread_cond_t *colorCond;
    pthread_cond_t *trafficFlipCond;
    pthread_cond_t *bridgeCond;
    int *bridgeEmptySpace;
    int bridgeCapacity;
    int *carsToPass;
    int color;
    int *turn;
}car_t;

void preBridge(car_t *arg){
    printf("Car going towards bridge (color: %d)\n", arg->color);
    pthread_mutex_lock(arg->mtx);
    
    //Wait if its not the cars turn
    while(*(arg->turn) != arg->color){
        printf("Car waiting for turn change (turn %d while color %d)\n", *(arg->turn), arg->color);
        pthread_cond_wait(arg->colorCond, arg->mtx);
    }
    //Wait if max cars
    while(*(arg->carsToPass) == MAX_CARS){
        printf("Car waiting to ensure fairness (cars already passed %d of color %d\n", *(arg->carsToPass), arg->color);
        pthread_cond_wait(arg->colorCond, arg->mtx);
    }
    //Wake new car
    //If car limit reached, the new car will wait
    //in while loop
    (*(arg->carsToPass))++;
    pthread_cond_signal(arg->colorCond);

    //Wait if bridge full
    while(*(arg->bridgeEmptySpace) == 0){
        printf("Car waiting for space in bridge (color: %d)\n", arg->color);
        pthread_cond_wait(arg->bridgeCond, arg->mtx);
    }
    //take up space in bridge
    (*(arg->bridgeEmptySpace))--;
    printf("bridgeEmptySpace: %d\n", *(arg->bridgeEmptySpace));

    pthread_mutex_unlock(arg->mtx);
    printf("Car travelling throught bridge (color: %d)\n", arg->color);
}

void postBridge(car_t *arg){
    printf("Car exiting bridge (color: %d)\n", arg->color);
    pthread_mutex_lock(arg->mtx);

    //allow new car to enter bridge
    pthread_cond_signal(arg->bridgeCond);
    (*(arg->bridgeEmptySpace))++;

    //If max cars and all cars exited bridge switch turns
    if(*(arg->carsToPass) == MAX_CARS){
        if(*(arg->bridgeEmptySpace) == arg->bridgeCapacity){

            (*(arg->turn)) = !(*(arg->turn));
            (*(arg->carsToPass)) = 0;
            printf("\n\n--FLIPPING TRAFFIC--\n\n");
            pthread_cond_signal(arg->trafficFlipCond);
        }
    }

    pthread_mutex_unlock(arg->mtx);
}

void *car(void *args){
    car_t *arg = (car_t*)args;

    preBridge(arg);

    sleep(TIME_TO_CROSS);

    postBridge(arg);

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
    int turn = BLUE;

    pthread_t carThread;

    pthread_mutex_t *mtx      = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *blueCond   = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *redCond    = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_t *bridgeCond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));

    pthread_mutex_init(mtx, NULL);
    pthread_cond_init(blueCond, NULL);
    pthread_cond_init(redCond, NULL);
    pthread_cond_init(bridgeCond, NULL);

    printf("Enter: |car colour| |num of new cars|\n");
    printf("Note: 0 for blue 1 for red, blue cars have first turn\n");
    while (scanf("%d %d", &newCarColour, &newCarNum) != EOF){
        for(int i = 0; i < newCarNum; i++){
            car_t *tmp = malloc(sizeof(car_t));

            if(newCarColour == BLUE){
                tmp->color = BLUE;
                tmp->colorCond = blueCond;
                tmp->trafficFlipCond = redCond;
            }
            else{
                tmp->color = RED;
                tmp->colorCond = redCond;
                tmp->trafficFlipCond = blueCond;
            }
            tmp->mtx = mtx;
            tmp->bridgeCond = bridgeCond;
            
            tmp->bridgeCapacity = bridgeCapacity;
            tmp->bridgeEmptySpace = &bridgeEmptySpace;
            tmp->carsToPass = &carsToPass;
            tmp->turn = &turn; 
            
            pthread_create(&carThread, NULL, car, tmp);
        }
    }
    free(mtx);
    free(blueCond);
    free(redCond);
    free(bridgeCond);
}
