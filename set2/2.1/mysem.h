#ifndef MYSEM
#define MYSEM

typedef struct mysem{
    int semid;
    int semidInternal;
    int initialized;
}mysem_t;

int mysem_init(mysem_t *s, int n);
int mysem_down(mysem_t *s);
int mysem_up(mysem_t *s);
int mysem_destroy(mysem_t *s);

#endif
