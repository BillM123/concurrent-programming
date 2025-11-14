#include <stdio.h>
#include <pthread.h>
#include "mypipe.h"
#define PIPE_SIZE 64

typedef struct thread_functions_args {
    int pipe_write_id;
    int pipe_read_id;
    char *write_filename;
    char *read_filename;    
    int complete;
}thread_args;


void * Thread1(void *args);
void * Thread2(void *args);

int main(int argc, char *argv[]){
    pthread_t t1, t2; 
    int id1,id2;
    thread_args t1_args;
    thread_args t2_args;

    t1_args.complete = 0;
    t2_args.complete = 0;

    if(argv[1] == NULL){
        printf("Need file as input.\n");
        return -1;
    }

    id1 = pipe_open(PIPE_SIZE);
    id2 = pipe_open(PIPE_SIZE);

    t1_args.pipe_write_id = id1;
    t1_args.pipe_read_id = id2;
    t1_args.read_filename = argv[1];
    t1_args.write_filename = "second_output.txt";

    t2_args.pipe_write_id = id2;
    t2_args.pipe_read_id = id1;
    t2_args.write_filename = "first_output.txt";
    t2_args.read_filename = "first_output.txt";
              
    pthread_create(&t1, NULL, Thread1, &t1_args);
    pthread_create(&t2, NULL, Thread2, &t2_args);

    while(t1_args.complete == 0 || t2_args.complete == 0){
        //Wait until both threads are done
    }

    return 0;
}

void * Thread1(void *args){
    thread_args *thread1_args = (thread_args*) args;
    FILE *readingFile;
    FILE *writingFile;
    char writing_char;
    char reading_char;
    int pipe_write_return = 1;
    int pipe_read_return = 1;

    readingFile = fopen(thread1_args->read_filename, "r");
    if(readingFile == NULL){
        perror("Error opening original file for reading");
        return NULL;
    }
  
    while((writing_char = fgetc(readingFile)) != EOF){
        pipe_write_return = pipe_write(thread1_args->pipe_write_id, writing_char);
        if(pipe_write_return == -1){
            printf("No such pipe!\n");
            return NULL;
        }
    }
    if (feof(readingFile)){
        pipe_writeDone(thread1_args->pipe_write_id);
    }
    fclose(readingFile);

    writingFile = fopen(thread1_args->write_filename, "w");
    if(writingFile == NULL){
        perror("Error opening 2nd copy for writing");
        return NULL;
    }
    while(pipe_read_return != 0){
        pipe_read_return = pipe_read(thread1_args->pipe_read_id, &reading_char);
        if(pipe_read_return == 0){
            break;
        }
        fprintf(writingFile,"%c", reading_char);
        if(pipe_read_return == -1){
            printf("No such pipe!\n");
            return NULL;
        }
    }
    fclose(writingFile);
    thread1_args->complete = 1;

    return NULL;
}

void * Thread2(void *args){
    thread_args *thread2_args = (thread_args*) args;
    FILE *readingFile;
    FILE *writingFile;
    char reading_char;
    char writing_char;
    int pipe_read_return = 1;
    int pipe_write_return = 1;

    writingFile = fopen(thread2_args->write_filename, "w");
    if(writingFile == NULL){
        perror("Error opening 1st copy for writing");
        return NULL;
    }
    do{
        pipe_read_return = pipe_read(thread2_args->pipe_read_id, &reading_char);
        if(pipe_read_return == 0){
            break;
        }
        fprintf(writingFile,"%c", reading_char);
        if(pipe_read_return == -1){
            printf("No such pipe!\n");
            return NULL;
        }
    }while(pipe_read_return != 0);
    fclose(writingFile);
    
    readingFile = fopen(thread2_args->read_filename, "r"); 
    if(readingFile == NULL){
        perror("Error opening 1st copy for reading");
        return NULL;
    }
    while((writing_char = fgetc(readingFile)) != EOF){
        pipe_write_return = pipe_write(thread2_args->pipe_write_id, writing_char);
        if(pipe_write_return == -1){
            printf("No such pipe!\n");
            return NULL;
        }
    }
    if (feof(readingFile)){
        pipe_writeDone(thread2_args->pipe_write_id);
    }
    fclose(readingFile);
    thread2_args->complete = 1;

    return NULL;
}
