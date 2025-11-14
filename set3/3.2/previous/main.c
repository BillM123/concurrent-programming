#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct targ{
    long num;
    int status;
}threadArg;

void *work(void *thread_arg);

int main(int argc, char *argv[]){
    if(argv[1] == NULL || atoi(argv[1]) <= 0){
        printf("Number of workers should be first argument\n");
        return -1;
    }

    //initialize variables
    int scanReturn = 0;
    int inputValue = 0;
    int canProceed = 0;
    int proccessesTerminated = 0;

    //create table for proccesses and arguments
    pthread_t *threadTable = (pthread_t*)malloc(atoi(argv[1])*sizeof(pthread_t)); 
    threadArg *argTable = (threadArg*)malloc(atoi(argv[1])*sizeof(threadArg));

    //create all workers
    for(int i = 0; i < atoi(argv[1]); i++){
        argTable[i].status = 0;
        argTable[i].num = 0;
        pthread_create(&threadTable[i], NULL, work, &argTable[i]);
    }

    //feed workers input while input is given
    do {
        scanReturn = scanf("%d", &inputValue);
        if(scanReturn != EOF){
            //searches proccesses repeatedly until one becomes available
            while(1){

                for(int i = 0; i < atoi(argv[1]); i++){
                    if(argTable[i].status == 0){
                        //Give process work and exit loops
                        argTable[i].num = inputValue;
                        argTable[i].status = 1;
                        canProceed = 1;
                        break;
                    }
                }
                if(canProceed == 1){
                    break;
                }

            }
        }
        canProceed = 0;
    }while (scanReturn != EOF);

    //order proccesses to terminate
    for(int i = 0; i < atoi(argv[1]); i++){
        argTable[i].status = -1;
    }

    //wait for proccesses to actualy terminate
    do {
        for(int i = 0; i < atoi(argv[1]); i++){
            if(argTable[i].status == -2){
                argTable[i].status = -3;
                proccessesTerminated ++;
            }
        }
    }while (proccessesTerminated == atoi(argv[1]));

    //cleanup
    free(threadTable);
    free(argTable);
    return 0;
}

void *work(void *thread_arg){
    int cnt = 0;
    threadArg *arg = (threadArg*)thread_arg;

    while (1) {
        while (arg->status == 0) {
        
        }
        if(arg->status == -1) break; //main ordered termination

        // Code concerning primality testing originates from geeksforgeeks:
        // https://www.geeksforgeeks.org/c/c-program-to-check-whether-a-number-is-prime-or-not/
        if (arg->num <= 1)
            printf("%ld is NOT prime\n", arg->num);
        else {

            // Check for divisors from 1 to n
            for (int i = 1; i <= arg->num; i++) {

                // Check how many number is divisible
                // by n
                if (arg->num % i == 0)
                    cnt++;
            }

            // If n is divisible by more than 2 numbers
            // then it is not prime
            if (cnt > 2)
                printf("%ld is NOT prime\n", arg->num);

            // else it is prime
            else
                printf("%ld is prime\n", arg->num);
        }

        if(arg->status == -1) break; //main ordered termination
        arg->status = 0;
        cnt = 0;
    }

    arg->status = -2; //program notifies main of termination
    return NULL;
}

// status =  1: thread is working (set by main)
// status =  0: thread is idling and waiting for work
// status = -1: thread is ordered to terminate by main
// status = -2: thread will terminate
// status = -3: main aknowledges termination
