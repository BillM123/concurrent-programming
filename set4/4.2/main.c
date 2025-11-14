#include <stdio.h>
#include <unistd.h>
#include "include/mythreads.h"

void routine(){    
    printf("Test from threads\n");
    sleep(3);
    printf("Ending thread\n");
}

int main(int argc, char* argv[]) {
    mythr_t t1,t2;
    
    printf("before init\n");
    mythreads_init();
    mythreads_init();
    printf("before create\n");
    mythreads_create(&t1, routine,NULL);
    mythreads_create(&t2, routine,NULL);
    printf("before join\n");
    mythreads_join(&t1);
    mythreads_join(&t2);
    
    mythreads_destroy(&t1);
    mythreads_destroy(&t2);

    return 0;
}
