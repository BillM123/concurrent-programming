#include "include/mythreads.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

//--------------------------------------------------------------------//
//--SCHEDULER: Everything related to the scheduler & signal handling--//
//--------------------------------------------------------------------//

mysched_t scheduler = {NULL}; //concider removing mysched_t, if it is comprised only of the Q

// This should be used to save the timer state, should the timer be frozen
// Could be a valid method to preserve critical sections, signal blocking
//  respects slightly more the round robin principle of fairness 

//struct itimerval savedTimer;

// This should be used to temporarily block new SIGALRM signals

sigset_t set, oldset;



// This function will handle the SIGALRM signal
// NOTE: should be passed to struct sigaction.sa_handler

void switchThread(){
    if(isEmpty(scheduler.thread_queue)) return;

    //printf("q\n");
    mythr_t *curr = dequeue(scheduler.thread_queue);
    // dequeue(scheduler.thread_queue, &curr);
    if(curr->state != FINISHED) enqueue(scheduler.thread_queue, curr); 
    else printf("Thread finished\n");
    //printf("!q\n");

    mythr_t *nxt = peek(scheduler.thread_queue);

    // Restore signal mask explicitly to prevent signal blocking issues
    sigset_t emptySet;
    sigemptyset(&emptySet);
    sigprocmask(SIG_SETMASK, &emptySet, NULL);

    //printf("(curr)Stack pointer (rsp): %p\n", curr->context.uc_stack.ss_sp);
    //printf("(nxt)Stack pointer (rsp): %p\n", nxt->context.uc_stack.ss_sp);
    //printf("sh: entered signal handler\n");

    swapcontext(&curr->context, &nxt->context);
}//


// This function initializes and configures the timer.
// Note that it will also be called from scheduler_init.
// Depending on mode, the timer might ring immediately or
//  first wait for TIMER ms, regardless it will always ring
//  with a period of TIME ms after that.
// A second call to this function may reset the timer

void timer_set(short mode){
    struct itimerval timer;
  
    timer.it_value.tv_sec = 0;
    //it amounts to eiter time or 1, if timer is 0 it will not activate
    timer.it_value.tv_usec = TIMER*(mode == SET_DELAYED_PERIOD) + !(mode == SET_DELAYED_PERIOD); 
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIMER; //should amount to 50 ms

    //mythr_t *curr = peek(scheduler.thread_queue);
    //printf("(timer_set):setting off, mode=%d  sp = %p\n", mode, curr->context.uc_stack.ss_sp);
    
    setitimer(ITIMER_REAL, &timer, NULL);
}


// This function initializes everything that has to do with the scheduler.
// Since this does also start the timer, this function should be called 
//  after every other initialization.

void scheduler_init(mythr_t *main){
    struct sigaction sa;

    //Initializes queue
    scheduler.thread_queue = createQueue();
    enqueue(scheduler.thread_queue, main);

    //Configs SIGALRM handling
    sa.sa_handler = switchThread;
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    //Prepares signal blocking mask
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);

    //Starts timer
    timer_set(SET_DELAYED_PERIOD);

}


// This function will block SIGALRM, until switch_unblock is called. 
// Use only on critical sections

void switch_block(){ 
    sigprocmask(SIG_BLOCK, &set, &oldset); 
}


// This function will unblock SIGALRM, if previously blocked

void switch_unblock(){
    sigprocmask(SIG_UNBLOCK, &oldset, NULL);
}


//----------------------------------------------------------------//
//--THREADS: Everything related to thread functions and contexts--//
//----------------------------------------------------------------//


//Not malloce'd because no way it can be free'd
mythr_t mainThr; 

//Included because the implementation requirements are ambiguous.
//Might be required, or user might be expected to call mythreads_destroy

void thread_exit_handler(){
    switch_block();
    printf("Frinishing\n");
    
    mythr_t *curr = peek(scheduler.thread_queue);
    //Will allow mythreads_join to work, also handler removes finished threads
    curr->state = FINISHED; 

    switch_unblock();

    //Even if sigalrm goes off here, thread will yield
    mythreads_yield();
}


//Threads that terminate will switch to this context

ucontext_t exit_handler;


void exit_handler_init() {
    getcontext(&exit_handler);

    exit_handler.uc_stack.ss_sp = malloc(STACK_SIZE); // currently not free'd
    exit_handler.uc_stack.ss_size = STACK_SIZE;
    exit_handler.uc_stack.ss_flags = 0;

    exit_handler.uc_link = NULL; // No further context to switch to

    makecontext(&exit_handler, thread_exit_handler, 0); // This sets up the function to run
}


//NOTE: It is (somewhat) unclear how the main thread should be handled
//      The safe assumption is that no other thread will try to "join"
//      the main thread

//Initializes threading process. Use this only once!

int mythreads_init(){
    mainThr.state = ACTIVE;

    //get main context 
    if(getcontext(&mainThr.context)) return -1;

    //Initializes exit_handler. Terminating threads will switch to that context
    exit_handler_init();

    //Init's: thread queue, signal handler, sigblocking, timer
    scheduler_init(&mainThr);
    
    return 0;
} 

int mythreads_create(mythr_t *thr, void (body)(void *), void *arg){
    //printf("(create)Creating new thread...\n");
    switch_block();
    //printf("(create)alrm is blocked\n");

    if(getcontext(&thr->context)) return -1; //If this fails prob everything will break

    thr->stack = malloc(STACK_SIZE);
    thr->context.uc_stack.ss_size = STACK_SIZE;
    thr->context.uc_stack.ss_sp = thr->stack;
    thr->context.uc_stack.ss_flags = 0;

    thr->context.uc_link = &exit_handler;

    makecontext(&thr->context, (void (*)(void))body, 1, arg);

    //->Consider adding wrapper function, to allow scheduler to be its own library
    //  (would allow for scheduler to be static in that library)
    //->Consider using switch_block here, if above is not critical section
    enqueue(scheduler.thread_queue, thr); //NOTE: Queue of mythr_t

    thr->state = ACTIVE;
    switch_unblock();

    return 0;
}

int mythreads_yield(){
    switch_block();
    // Might lose priority if alarm goes off while in timer_set,
    // same in sleep, consider wrapping this in switch block (did)
    //mythr_t *curr = peek(scheduler.thread_queue);
    //printf("(yield):switching..., sp = %p\n", curr->context.uc_stack.ss_sp);
    timer_set(SET_DELAYED_PERIOD);  

    // sigset_t pending;
    // sigpending(&pending);
    // int p = sigismember(&pending, SIGALRM);

    //curr = peek(scheduler.thread_queue);
    //printf("(yield):alarm going off..., sp = %p\n", curr->context.uc_stack.ss_sp);

    kill(getpid(), SIGALRM);
    switch_unblock();                 

    return 0;
}

int mythreads_sleep(int secs){
    //printf("(sleep):sleeping...\n");
    for(int start = time(NULL), curr = start; curr - start < secs; curr = time(NULL)){
        //printf("(sleep):Calling yield...\n");
        mythreads_yield(); //could be mythreads_yield, does not matter (well it now is)
    }
    return 0;
}

int mythreads_join(mythr_t *thr){
    while(thr->state != FINISHED){
        mythreads_yield();
    }
    return 0;
} 

int mythreads_destroy(mythr_t *thr){
    if(thr->state != FINISHED) return -1;

    thr->context.uc_stack.ss_sp = NULL;
    free(thr->stack);
    
    return 0;
} // Thread should already be removed from scheduler, if not finished function fails


//------------------------------------------------//
//--SEMAPHORES: Everything related to semaphores--//
//------------------------------------------------//


int mythreads_sem_create(mysem_t *s, int val){
    switch_block();
    if(s->initialized == 1)return -1;
    s->queue = createQueue();
    //switch_unblock could propably be here

    s->val = val;
    s->initialized = 1;

    switch_unblock();
    return 0;
}

int mythreads_sem_down(mysem_t *s){
    switch_block();
    if(s->initialized != 1)return -1;
    // Assumes scheduler not empty: function could not be called otherwise
    mythr_t *curr = peek(scheduler.thread_queue);

    if(s->val == 0){
        enqueue(s->queue, curr);
        curr->state = WAITING; 
        switch_unblock();

        while(curr->state == WAITING) mythreads_yield();
    }
    else{
        s->val--;
        switch_unblock();
    }
    return 0;
}

int mythreads_sem_up(mysem_t *s){
    switch_block();
    if(s->initialized != 1)return -1;

    if(!isEmpty(s->queue)){
        mythr_t *waitingThread = dequeue(s->queue);
        // dequeue(s->queue, waitingThread);
        waitingThread->state = ACTIVE;
    }
    else{
        s->val++;
    }
    switch_unblock();
    return 0;

}

int mythreads_sem_destroy(mysem_t *s){
    switch_block();

    if(s->initialized != 1)return -1;
    if(!isEmpty(s->queue))return -1;
    
    freeQueue(s->queue);
    s->initialized = 0;
    
    switch_unblock();
    return 0;
} //Fails if any thread is waiting in queue
