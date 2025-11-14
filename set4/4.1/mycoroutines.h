#ifndef COROUTINES
#define COROUTINES

//include
#include <signal.h>
#include <ucontext.h>

//definitions
#define STACK_SIZE 65536

//Structs and datatypes
typedef struct corroutine{
    ucontext_t context;
    char *stack;
}co_t;

//Coroutine function definitions
int mycoroutines_init(co_t *main);
int mycoroutines_create(co_t *co, void (body)(void *), void *arg);
int mycoroutines_switchto(co_t *co);
int mycoroutines_destroy(co_t *co);

#endif
