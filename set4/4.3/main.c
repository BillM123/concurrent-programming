#include <stdio.h>
#include <stdlib.h>
#include "include/mythreads.h"

typedef struct function_args{
    mysem_t *mtx;
    mysem_t *wq, *rq;//waiting queues for readers and writers 
    int *w_wait;//number of waiting writers in queue
    int *r_wait;//number of waiting readers in queue
    int *wr_cs;//number of writers in critical section
    int *rd_cs;//number of readers in critical section
    int *wr_id;
    int *rd_id;
    char *buf;
}functionArg;


// Writer thread function
void writer(void *args) {
    printf("enter writer\n");
    functionArg *writer_args = (functionArg*)args;

    //down the semaphore used as a mutex for mutual exclusion
    mythreads_sem_down(writer_args->mtx);
    //checking if there are readers or another writer into the critical section
    if ((*(writer_args->rd_cs) + *(writer_args->wr_cs)) > 0) {
        //writer enters the writers queue
        *(writer_args->w_wait) = *(writer_args->w_wait) + 1;
        //up the semaphore for mutual exclusion
        mythreads_sem_up(writer_args->mtx);
        printf("(Writer): Writer %d waiting on writers queue\n", *(writer_args->wr_id));
        mythreads_sem_down(writer_args->wq);
    } else {
        //writer enters critical section
        printf("(Writer): Writer %d entering CS\n", *(writer_args->wr_id));
        *(writer_args->wr_cs) = *(writer_args->wr_cs)+1;
        //up the semaphore for mutual exclusion
        mythreads_sem_up(writer_args->mtx);
    }
    // Write operation,  write data in buffer
    printf("(Writer): Writer %d over writing content: -",*(writer_args->wr_id));
    scanf("%s",writer_args->buf);
    //down the semaphore used as a mutex for mutual exclusion
    mythreads_sem_down(writer_args->mtx);
    //writer gets out of critical section
    printf("(Writer): Writer %d got out of CS\n",*(writer_args->wr_id));
    *(writer_args->wr_cs) = *(writer_args->wr_cs)-1;
    //Check for readers waiting on the readers queue 
    if (*(writer_args->r_wait) > 0) {
        printf("(Writer): Reader %d waking from readers queue\n", *(writer_args->rd_id));
        (writer_args->r_wait)--;
        printf("(Writer): Reader %d entering CS\n", *(writer_args->rd_id));
        (writer_args->rd_cs)++;
        //wake up a reader from readers queue
        mythreads_sem_up(writer_args->rq);
    } else {
        //checkfor writers waiting on the writers queues
        if (*(writer_args->w_wait) > 0) {
             printf("(Writer): Writer %d waking from writers queue\n", *(writer_args->wr_id));
            *(writer_args->w_wait) = *(writer_args->w_wait)-1;
            printf("(Writer): Writer %d entered CS\n", *(writer_args->wr_id));
            *(writer_args->wr_cs) = *(writer_args->wr_cs)+1;
            //wake up writer from writers queues
            mythreads_sem_up(writer_args->wq);
        }
    }
    //up the semaphore for mutual exclusion
    mythreads_sem_up(writer_args->mtx);
    return;
}

// Reader thread function
void reader(void *args) {
    printf("enter reader\n");
    functionArg *reader_args = (functionArg*)args;

    //down the semaphore used as a mutex for mutual exclusion
    mythreads_sem_down(reader_args->mtx);
    //Check if there are any writers into the critical section or writers on the writers queue
    if ((*(reader_args->wr_cs) + *(reader_args->w_wait)) > 0) {
        //reader enters readers queue
        *(reader_args->r_wait) = *(reader_args->r_wait) +1;
        //up the semaphore for mutual exclusion
        mythreads_sem_up(reader_args->mtx);
        printf("(Reader): Reader %d waiting on readers queue\n", *(reader_args->rd_id));
        mythreads_sem_down(reader_args->rq);
        //check for readers on readers queue
        if(*(reader_args->r_wait) > 0){
            //reader wakes up fro m readers queue
            *(reader_args->r_wait)= *(reader_args->r_wait)-1;
            printf("(Reader): Reader %d waking from readers queue\n", *(reader_args->rd_id));
            *(reader_args->rd_cs) = *(reader_args->rd_cs)+1;
            printf("(Reader): Reader %d entering CS\n", *(reader_args->rd_id));
            mythreads_sem_up(reader_args->rq);
        }
        else {
            //up the semaphore for mutual exclusion
            mythreads_sem_up(reader_args->mtx);
        }
    } 
    else {
        //reader enters critical section
        *(reader_args->rd_cs) = *(reader_args->rd_cs)+1;
        printf("(Reader): Reader %d entered CS\n", *(reader_args->rd_id));
        //up the semaphore for mutual exclusion
        mythreads_sem_up(reader_args->mtx);
    }
    //Reading operation
    printf("(Reader): Reader %d reading content: -%s\n",*(reader_args->rd_id),reader_args->buf);
    //down the semaphore used as a mutex for mutual exclusion
    mythreads_sem_down(reader_args->mtx);
    *(reader_args->rd_cs) = *(reader_args->rd_cs)-1;
    printf("(Reader): Reader %d got out of CS\n", *(reader_args->rd_id));
    //check if there are not readers into critical section and also there are readers on readers queue
    //if yes give priorityy to writers
    if ((*(reader_args->rd_cs) == 0) && (*(reader_args->w_wait) > 0)) {
        //Wake up writer from writers queue
        printf("(Reader): Writer %d waking from writers queue\n", *(reader_args->wr_id));
        *(reader_args->w_wait) = *(reader_args->w_wait)-1;
        printf("(Reader): Writer %d entered CS\n", *(reader_args->wr_id));
        *(reader_args->wr_cs) =*(reader_args->wr_cs)+1;
        mythreads_sem_up(reader_args->wq);
    }
    //up the semaphore for mutual exclusion
    mythreads_sem_up(reader_args->mtx);
    return;
}

int main() {
    mysem_t mtx;//mutual exclusion for readers and writers
    mysem_t wq, rq;//waiting queues for readers and writers 
    int w_wait = 0;//number of waiting writers in queue
    int r_wait = 0;//number of waiting readers in queue
    int wr_cs = 0;//number of writers in critical section
    int rd_cs = 0;//number of readers in critical section
    int rds_num;//number of readers set by user
    int wrs_num;//number of writers set by user 
    int rd_id =0;
    int wr_id = 0;
    char *buffer = NULL;

    //create and initialize the semaphores
    mythreads_sem_create(&mtx,1);
    mythreads_sem_create(&wq, 0);
    mythreads_sem_create(&rq, 0);

    
    functionArg *args = malloc(sizeof(functionArg));
    buffer = malloc(101*sizeof(char));
    args->mtx = &mtx;
    args->rq = &rq;
    args->wq = &wq;
    args->w_wait = &w_wait;
    args->r_wait =  &r_wait;
    args->wr_cs = &wr_cs;
    args->rd_cs = &rd_cs;
    args->rd_id = &rd_id;
    args->wr_id = &wr_id;

    printf("Enter number of writers:\n");
    scanf("%d", &wrs_num);
    while (wrs_num <= 0){
        printf("Number of writers must be a positive value\n");
        printf("Enter number of writers:\n");
        scanf("%d", &wrs_num);
    }
    mythr_t *writers = (mythr_t*)malloc(wrs_num*sizeof(mythr_t)); 
    printf("Enter number of readers:\n");
    scanf("%d", &rds_num);
    while (rds_num <= 0){
        printf("Number of readers must be a positive value\n");
        printf("Enter number of readers:\n");
        scanf("%d", &rds_num);
    }
    mythr_t *readers = (mythr_t*)malloc(rds_num*sizeof(mythr_t));

    printf("Number of writers is %d, number of readers is %d\n", wrs_num, rds_num);

    printf("(Main): writing content (max 100 characters): -");
    scanf(" %100s", buffer);
    args->buf = buffer;
    printf("buffer is %s \n", args->buf);
    mythreads_init();

    //Creating threads for readers and writers 
    for (int i = 0; i < wrs_num; i++) {
            *(args->wr_id) = i+1;
            mythreads_create(&writers[i], writer, args);
    }
    for (int j=0; j < rds_num; j++){
        *(args->rd_id) = j+1;
        mythreads_create(&readers[j], reader, args);
    }
    
    for (int i = 0; i < wrs_num; i++) {
        mythreads_join(&writers[i]);
    }
    for (int j = 0; j < rds_num; j++) {
        mythreads_join(&readers[j]);
    }
    
    free(buffer);
    free(writers);
    free(readers);
    free(args);
    return 0;
}
 
