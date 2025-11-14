#ifndef FIFO
#define FIFO

//function prototypes for fifo pipe
int pipe_open(int size);
int pipe_write(int p, char c); 
int pipe_writeDone(int p);
int pipe_read(int p, char *c);

//datatypes
typedef struct ringBuffer{
    char* buf;
    //int bufUsed;
    int readPos;
    int writePos;
    int bufSize;
}ringBufferT ;

typedef struct pipe{
    int id;
    ringBufferT buffer;
    short writeDone;
}pipeT;

//definitions
#define MAX_PIPES 32

#endif
