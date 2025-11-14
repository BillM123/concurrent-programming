#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include"mysem.h"

#define N 500

    typedef struct parametroi{
        int id;
        int* value;
        mysem_t *s1;
    }arguments;

    void *increase(void* Args){
        arguments *args = (arguments*) Args;
        int ret_up, ret_down;

        printf("This is thread %d using down\n", args->id);
        ret_down = mysem_down(args->s1);

        if(ret_down == -1){
            printf("mysem_down failed\n");
        }
        else{
            (*(args->value))++;
            printf("curent value is %d\n", *args->value);
        }

        printf("This is thread %d using up\n", args->id);
        ret_up = mysem_up(args->s1);
        
        if(ret_up == -1){
            printf("mysem_up failed\n");
        }
        
        return NULL;
    }

    void *double_increase(void* Args){
        arguments* args = (arguments*) Args;
        int ret_up;

        printf("This is thread %d using up\n", args->id);
        //The value of the Semaphore is already 1
        ret_up = mysem_up(args->s1);

        if(ret_up == -1){
            printf("mysem_up failed\n");
        }

        return NULL;
    }

int main(int argc, char *argv[]){
    pthread_t thread_table[N], special[2];
    int test_val, i, ret_init, ret_destroy;
    mysem_t s;
    arguments *args = (arguments*)malloc((N)*sizeof(arguments));

    //Initializing the arguments
    for(int i = 0; i < N; i++){
        test_val = 0;
        args[i].value = &test_val;
        args[i].s1 = &s;
        args[i].s1->initialized = 0;
    }

    printf("Calling the function increase without initializing\n");
    if(pthread_create(&special[0], NULL, increase, &(args[0])) == -1){
        printf("problem with the first create\n");
    }
    else{
        if(pthread_join((special[0]), NULL) == -1){
            printf("problem with the first join\n");
        }
    }

    //initializing for the first time
    for(int i = 0; i < N; i++){
        ret_init = mysem_init((args[i].s1), 1);
        if(ret_init == 0){
            printf("N should be either zero or one\n");
        }
        else{
            if(ret_init == -1){
                printf("Semaphore already initialized\n");
            }
        }
    }
    //trying to initialise again
    ret_init = mysem_init((args[0].s1), 1);
    if(ret_init == 0){
        printf("N should be either zero or one\n");
    }
    else{
        if(ret_init == -1){
            printf("Semaphore already initialized\n");
        }
    }

    for(i = 1; i < N-1; i++){
        args[i].id = i;
        //calling the increase function N times
        if(pthread_create(thread_table + i, NULL, increase, &(args[i])) == -1){
            printf("%d create failed\n", i);
            return -1;
        }
    }

    //We use a differnt for loop because otherwise we wouldn't have conceruncy
    for(i = 0; i < N; i++){
        if(pthread_join(thread_table[i], NULL) == -1){
            printf("%d join failed\n", i);
            return -1;
        }
    }

    //Here we expect the value to be N-1
    printf("The value of test_val = %d\n", test_val);

    pthread_create((special + 1), NULL, double_increase, &(args[N-1]));
    pthread_join((special[1]), NULL);

    for(int i = 0; i < N; i++){
        //Destroy all the Semaphores
        ret_destroy = mysem_destroy(((args + i)->s1));
        if(ret_destroy == -1){
            printf("Semaphore already initialized\n");
        }
    }

    free(args);
    
    return 0;
}
