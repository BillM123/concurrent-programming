#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mycoroutines.h"

#define BUF_SIZE 64

#define newstring(name, oldstring, cont) \
    char *name = malloc((strlen(oldstring)+strlen(cont)+1)*sizeof(char)); \
    strcpy(name, oldstring); \
    strcat(name, cont);

typedef struct func_args{
    char *filename;
    char *buf;
    int *occ;
    co_t *target;
}func_args;

void func1(void *arg){
    func_args *args = (func_args*) arg;
    FILE *copy;
    char ch = '\0';

    newstring(copy1, args->filename, ".copy1");
    copy = fopen(copy1, "w");

    while(1){
        for(int i = 0; i < *(args->occ); i++){
            fprintf(copy, "%c", args->buf[i]);
        }
        if(*(args->occ) != BUF_SIZE) break;
        mycoroutines_switchto(args->target); 
    }

    fclose(copy);
    copy = fopen(copy1, "r");

    *(args->occ) = 0;
    ch = fgetc(copy);
    for(*(args->occ) = 0; ch != EOF; ch = fgetc(copy), (*(args->occ))++){
        if(*(args->occ) == BUF_SIZE){
            mycoroutines_switchto(args->target); 
            *(args->occ) = 0;
        }
        args->buf[*(args->occ)] = ch;
    }
    mycoroutines_switchto(args->target); 

    fclose(copy);
    free(copy1);
}

void func2(void *arg){
    func_args *args = (func_args*) arg;
    FILE *original, *copy;
    char ch = '\0';

    newstring(copy2, args->filename, ".copy2");
    original = fopen(args->filename, "r");

    ch = fgetc(original);
    for(*(args->occ) = 0; ch != EOF; ch = fgetc(original), (*(args->occ))++){
        if(*(args->occ) == BUF_SIZE){
            mycoroutines_switchto(args->target);  
            *(args->occ) = 0;
        }
        args->buf[*(args->occ)] = ch;
    }

    fclose(original);
    copy = fopen(copy2, "w");
    mycoroutines_switchto(args->target); 

    while(1){
        for(int i = 0; i < *(args->occ); i++){
            fprintf(copy, "%c", args->buf[i]);
        }
        if(*(args->occ) != BUF_SIZE) break;
        mycoroutines_switchto(args->target); 
    }

    fclose(copy);
    free(copy2);
}


int main(int argc, char **argv){
    if(argv[1] == NULL){
        printf("Need file as input.\n");
        return -1;
    }
    char *buf = malloc(BUF_SIZE*sizeof(char));
    
    co_t main, co1, co2;
    int occ = 0;
    func_args a1 = {argv[1], buf, &occ, &co2}, 
              a2 = {argv[1], buf, &occ, &co1};
    

    mycoroutines_init(&main);

    mycoroutines_create(&co1, func1, &a1);
    mycoroutines_create(&co2, func2, &a2);

    printf("swapmain\n");
    mycoroutines_switchto(&co2);

    printf("swapclean\n");
    mycoroutines_switchto(&co1);

    mycoroutines_destroy(&co1);
    mycoroutines_destroy(&co2);


    free(buf);

}