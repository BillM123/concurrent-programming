#ifndef COROUTINES
#define COROUTINES

//include
#include <signal.h>
#include <ucontext.h>
#include "queue.h"

//definitions
#define STACK_SIZE 655360
#define EXIT_STACK_SIZE 4096
#define TIMER 750000

//Structs and datatypes
enum timer_mode{SET_IMMEDIATE, SET_DELAYED_PERIOD};

enum thread_state{ACTIVE, WAITING, FINISHED};

typedef struct mythread{
    ucontext_t context;
    char *stack;
    int state;
}mythr_t;

typedef struct mysemaphores{
    int semid;
    int initialized;
    int val;
    Queue *queue;
}mysem_t; // could propably be simply an id, and have struct details
          // be private.

typedef struct myscheduler{
    Queue *thread_queue;
}mysched_t;

//Thread function definitions

//Initializes thread environment
int mythreads_init();

//Creates new thread. Function pointed to by body should expect one 
//argument of type "void". New thread will be pointed to by *thr. 
int mythreads_create(mythr_t *thr, void (body)(void *), void *arg);

//Switches to the next available thread.
int mythreads_yield();

//Waits for a time interval of "secs".
int mythreads_sleep(int secs);

//Blocks while waiting the thread pointed to by *thr to terminate.
int mythreads_join(mythr_t *thr); 

//Destroys thread pointed to by *thr.
int mythreads_destroy(mythr_t *thr);

//Creates and initializes one semaphore, with value equal to val. 
int mythreads_sem_create(mysem_t *s, int val);

//Reduces value of the semaphore by one. If the semaphore value
//equals to zero, the function will block until up is called.
int mythreads_sem_down(mysem_t *s);

//Increases value of the semaphore, or unblocks one blocked down
//call if it exists.
int mythreads_sem_up(mysem_t *s); 

//Destroys the semaphore pointed to by s.
int mythreads_sem_destroy(mysem_t *s); 

#endif
