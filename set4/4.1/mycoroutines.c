#include <stdio.h>
#include "mycoroutines.h"
#include <stdlib.h>

static co_t *currentContext, *mainContext;

int mycoroutines_init(co_t *main){
    
    if(getcontext(&main->context)){
        printf("Error in initialization\n");
        return -1;
    }
    mainContext = main;
    currentContext = main;
    return 0;
} 

int mycoroutines_create(co_t *co, void (body)(void *), void *arg){

    if(getcontext(&co->context)) return -1;

    co->stack = malloc(STACK_SIZE);
    co->context.uc_stack.ss_size = STACK_SIZE;
    co->context.uc_stack.ss_flags = 0;
    co->context.uc_stack.ss_sp = co->stack;

    //Note: main context is the only context guaranteed to be active
    co->context.uc_link = &mainContext->context;

    makecontext(&co->context, (void (*)(void))body, 1, arg);
    return 0;
}

int mycoroutines_switchto(co_t *co){
    co_t *previousContext = currentContext;
    currentContext = co;
    if(swapcontext(&previousContext->context, &co->context)) return -1;

    return 0;
}

int mycoroutines_destroy(co_t *co){
    co->context.uc_stack.ss_sp = NULL;
    
    free(co->stack);
    return 0;
}
