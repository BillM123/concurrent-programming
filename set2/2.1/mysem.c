#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <errno.h>
#include "mysem.h"

/*************************************************************************
 **************** Custom binary semaphore implementation: ****************
 * This implementation uses two general semaphores to keep track of the  *
 * value of our binary semaphore (one blocking and one non-blocking), so *
 * we dont need to use another synchronisation primitive. The values of  *
 * our two general semaphores combined will be always equal to one       *
 *************************************************************************
 */

//Init semaphore as binary
//Note: assumes semget is already used
//      to ensure correct key handling
int mysem_init(mysem_t *s, int n){
    if((n != 0) && (n != 1)){
        return 0;
    }
    if(s->initialized){
        printf("Semaphore is already initialized\n");
        return -1;
    }

    s->semid = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    s->semidInternal = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if(semctl(s->semid, 0, SETVAL, n) == -1){
        perror("semctl failed\n");
    }
    if(semctl(s->semidInternal, 0, SETVAL, 1-n) == -1){
        perror("semctl failed\n");
    }
    s->initialized = 1;
    return 1;
}


int mysem_down(mysem_t *s){
    if(s->initialized == 0){
        return -1;
    }

    struct sembuf ops = {0, -1, 0};
    int ret;
    do{
        ret = semop(s->semid, &ops, 1);
    }while (ret == -1 && errno == EINTR);

    if(ret == -1){
        perror("Error on down (first call)");
        return(-1);
    }

    ops.sem_op = 1;
    do{
        ret = semop(s->semidInternal, &ops, 1);
    }while (ret == -1 && errno == EINTR);

    if(ret == -1){
        perror("Error on down (second call)");
        return(-1);
    }
    return 1;
}


int mysem_up(mysem_t *s){
    if(s->initialized == 0){
        return -1;
    }
    //IPC_NOWAIT prevents blocking if we try to increase the binary semaphore
    //to a value greater than one
    struct sembuf ops = {0, -1, IPC_NOWAIT};
    //nsops = 1: sembuf is a table, currently one op at a time is supported
    int ret = semop(s->semidInternal, &ops, 1);

    if(ret == -1){
        if(errno == EAGAIN){ //assumes that errno is thread local
            fprintf(stderr, "Semaphore is already one\n");
            return 0;
        }
        perror("Error on up (first call)");
        return(-1);
    } 

    ops.sem_op = 1;
    ops.sem_flg = 0;
    ret = semop(s->semid, &ops, 1);

    if(ret == -1){
        fprintf(stderr,"Error on up (second call)\n");
        return(-1);
    }
    return 1;
}

//Destroys a semaphore from a group of semaphores
//Note: specific semnum should NOT be reused
int mysem_destroy(mysem_t *s){
    if(s->initialized == 0){
        return -1;
    }
    s->initialized = 0; //assumes no two threads will try to destroy sem simultaneously

    //Note: assumes that semctl will return 0 on success
    int ret = semctl(s->semid, 0, IPC_RMID);
    ret += semctl(s->semidInternal, 0, IPC_RMID);

    if(ret < 0){
        perror("");
        return(-1);
    }
    return 0;
}
