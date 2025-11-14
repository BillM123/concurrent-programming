#include <bits/pthreadtypes.h>
#ifndef MYSEM
#define MYSEM

typedef struct mysem{
    int val;
    int initialized;
    int semid;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
}mysem_t;

int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(mysem_t *s);

#endif
