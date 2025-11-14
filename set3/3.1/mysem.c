#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include "mysem.h"


//Init semaphore as binary
//Note: assumes semget is already used
//      to ensure correct key handling
int mysem_init(mysem_t *s, int n){
    if((n != 0) && (n != 1)){//The only valid values for binary semaphore are 0 and 1  
        printf("WRONG VALUE!!!\nn should be 0 or 1\n");
        return 0;
    }
    
    if(s->initialized){//Check if semaphore has already initialized -> the flag initializqed is 1 
        printf("Semaphore is already initialised\n");
        return -1;
    }

    //Give id to the semaphore 
    s->semid = rand();

    s->val = n;
    pthread_mutex_init(&s->mtx, NULL);
    pthread_cond_init(&s->cond,NULL);
    s->initialized = 1;//Set to 1 the flag that shows that the semaphore has been initialized
    return 1;
}


int mysem_down(mysem_t *s){
    if(s->initialized == 0){//Check if semaphore is not initialized 
        printf("The semaphore hasn't been initialized\n");
        return -1;
    }

    pthread_mutex_lock(&s->mtx);
        while(s->val == 0){
            pthread_cond_wait(&s->cond, &s->mtx);
        }
        s->val--;
    pthread_mutex_unlock(&s->mtx);

    return 1;   
}


int mysem_up(mysem_t *s){
    if(s->initialized == 0){//Check if semaphore is not initialized 
        printf("This semaphore is not initialised\n");
        return -1;
    }
    pthread_mutex_lock(&s->mtx);
        if(s->val == 0){
            s->val++;
        }
        else if(s->val == 1){
            pthread_mutex_unlock(&s->mtx);
            return 0;
        }
    pthread_mutex_unlock(&s->mtx);
    pthread_cond_signal(&s->cond);
    return 1;
}

//Destroys a semaphore from a group of semaphores
//Note: specific semnum should NOT be reused
int mysem_destroy(mysem_t *s){ 

    if(s->initialized == 0){//Check if semaphore isn not initialized 
        printf("This semaphore is not initialised\n");
        return -1;
    }
    s->initialized = 0; 
    s->semid = -1;
    s->val = -10;
    pthread_mutex_destroy(&s->mtx);
    pthread_cond_destroy(&s->cond);
    return 0;
}
