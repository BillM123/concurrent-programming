#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mypipe.h"

static pipeT *pipeArray[MAX_PIPES] = { [0 ... MAX_PIPES-1] = NULL };

void printBuffer(char *buf, int size){
    for(int i = 0; i <= size; i++){
        printf("%c ", buf[i]);
    }printf("\n");
}

int pipe_open(int size){
    pipeT *pipe;
    void *tmp = (char*)malloc(size); //should be adopted by buffer

    //initialize pipe
    pipe = (pipeT*)malloc(sizeof(pipeT));
    for(int i = 0; i < MAX_PIPES; i++){
        if(pipeArray[i] == NULL){
            pipeArray[i] = pipe;
            pipe->id = i;
            break;
        }
    }
    //init buffer
    pipe->buffer.buf = tmp;
    pipe->buffer.readPos = 0;
    pipe->buffer.writePos = 0;
    pipe->buffer.bufSize = size/sizeof(char);
    //init rest of pipe
    pipe->writeDone = 0;
    
    return pipe->id;
}

int pipe_write(int p, char c){
    pipeT *pipe = pipeArray[p]; 
    
    if(pipe == NULL){
        return -1;
    } 
    while(pipe->buffer.writePos == pipe->buffer.readPos-1
        ||(pipe->buffer.writePos == pipe->buffer.bufSize-1 && pipe->buffer.readPos == 0)){
        //freeze if buffer is full   
    }
    pipe->buffer.buf[pipe->buffer.writePos] = c;

    if(pipe->buffer.writePos == pipe->buffer.bufSize-1){
        pipe->buffer.writePos = 0;
    }
    else{
        pipe->buffer.writePos += 1;
    }
    return 1;
}

int pipe_writeDone(int p){
    pipeT *pipe = pipeArray[p];
    if(pipe == NULL){
        return -1;
    }
    pipe->writeDone = 1;
    while(pipeArray[p] != NULL){
        
    }
    return 1;
}

int pipe_read(int p, char *c){
    pipeT *pipe = pipeArray[p];

    if(pipe==NULL){  
        return -1;
    }
    while(pipe->buffer.readPos == pipe->buffer.writePos){
        //prevent exiting if frozen here while write writes
        if(pipe->writeDone == 1 && pipe->buffer.readPos == pipe->buffer.writePos){  
            free(pipe->buffer.buf);
            free(pipe);
            pipeArray[p] = NULL;
            return 0;
        }
    }
    *c =  pipe->buffer.buf[pipe->buffer.readPos];

    if(pipe->buffer.readPos == pipe->buffer.bufSize-1){
        pipe->buffer.readPos = 0;
    }
    else{
        pipe->buffer.readPos += 1;    
    }
    return 1;
}
