#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUF_SIZE 64

typedef struct targ{
    char *name; //fileame
    int len; //length of the area we are intrested in for each mergesort iteration
    int pos; //keep track of where the pointer is (from 0 to len-1)
    int offset; //how far away the area we're intrested in is away from the start (convert to bytes when using it with read/write)
    int status; //Thread execution status: expect 0 while thread is running, 1 if thread finishes
}threadArg;


// Function: safe_write
// Description: Writes 'count' bytes from 'buffer' to file descriptor 'fd', ensuring that all bytes are written.
// Parameters:
//    - fd: File descriptor to write to.
//    - buffer: Pointer to the data to be written.
//    - count: Number of bytes to write.
// Returns:
//    - On success, returns the total number of bytes written (should be equal to 'count').
//    - On failure, returns -1 and sets errno appropriately.
ssize_t safe_write(int fd, const void *buffer, size_t count) {
    const char *buf = (const char *)buffer;
    size_t bytes_left = count;
    ssize_t bytes_written;
    size_t total_written = 0;

    while (bytes_left > 0) {
        bytes_written = write(fd, buf + total_written, bytes_left);

        if (bytes_written < 0) {
            // If the write call was interrupted by a signal, retry
            if (errno == EINTR) {
                continue;
            } else {
                // For other errors, return -1
                return -1;
            }
        }

        // If zero bytes were written, something went wrong, return failure
        if (bytes_written == 0) {
            break;
        }

        // Update the number of bytes left to write and the total bytes written
        bytes_left -= bytes_written;
        total_written += bytes_written;
    }

    return total_written;
}

// Function: safe_read
// Description: Reads 'count' bytes from file descriptor 'fd' into 'buffer', ensuring that all bytes are read.
// Parameters:
//    - fd: File descriptor to read from.
//    - buffer: Pointer to the buffer where the data will be stored.
//    - count: Number of bytes to read.
// Returns:
//    - On success, returns the total number of bytes read (which may be less than 'count' if EOF is reached).
//    - On error, returns -1 and sets errno appropriately.
ssize_t safe_read(int fd, void *buffer, size_t count) {
    char *buf = (char *)buffer;
    size_t bytes_left = count;
    ssize_t bytes_read;
    size_t total_read = 0;

    while (bytes_left > 0) {
        bytes_read = read(fd, buf + total_read, bytes_left);

        if (bytes_read < 0) {
            // If the read call was interrupted by a signal, retry
            if (errno == EINTR) {
                continue;
            } else {
                // For other errors, return -1
                return -1;
            }
        }

        // If zero bytes were read, we have reached EOF
        if (bytes_read == 0) {
            break;
        }

        // Update the number of bytes left to read and the total bytes read
        bytes_left -= bytes_read;
        total_read += bytes_read;
    }

    return total_read;
}

// Function to dump the contents of a binary file to a human-readable text file
int dumpBinaryToText(const char *binaryFileName, const char *textFileName) {
    FILE *binaryFile = fopen(binaryFileName, "rb");
    if (binaryFile == NULL) {
        perror("Error opening the binary file");
        return -1; 
    }

    FILE *textFile = fopen(textFileName, "w");
    if (textFile == NULL) {
        perror("Error opening the text file");
        fclose(binaryFile);
        return -1; 
    }

    int number;
    // Read integers from the binary file and write them to the text file
    while (fread(&number, sizeof(int), 1, binaryFile) == 1) {
        fprintf(textFile, "%d\n", number);
    }

    // Close both files
    fclose(binaryFile);
    fclose(textFile);

    //printf("Successfully dumped the contents of %s to %s\n", binaryFileName, textFileName);
    return 0;
}

int compare(const void* a, const void* b){
    int intA = *(const int *)a;
    int intB = *(const int *)b;
    return intA - intB;
}

//Function to save numbers to the file
//Note: The function uses a buffer to avoid constant shifting
int save(int fdStart, int fdTarget, threadArg *threadArg1, threadArg *fileInfo, int *buf, int *bufUsed, int num, int flush){
    int tmp;
    int writePos = fileInfo->offset+fileInfo->pos;
    int arg1PosRelative = (threadArg1->pos+1)-fileInfo->pos;//pos+1 potential issue?

    buf[*bufUsed] = num;
    (*bufUsed)++;

    //shift and write if buffer is full
    if(*bufUsed == MAX_BUF_SIZE || flush == 1){
        //Shift all numbers, to prevent ovewriting unread nums after threadArg1->pos
        //Note: We shift the numbers so the individual buckets will remain sorted
        int arg2PosRelative = *bufUsed - arg1PosRelative;

        //Keep current ofsets, and init new ones for the loop
        off_t target = lseek(fdStart, 0, SEEK_CUR);
        off_t endPos = lseek(fdTarget, 0, SEEK_CUR); //fd2 should return here
        off_t DupPos = lseek(fdTarget, endPos - 1*sizeof(int), SEEK_SET);
        off_t startPos = lseek(fdStart, DupPos - (arg2PosRelative)*sizeof(int), SEEK_SET);

        //Note: the gap between DupPos and startPos should be the number of bytes that would otherwise be overwritten
        if(flush != 1){
            //bytes behind target-sizeof(int) are "saved" into the buffer, so they
            //wont be ovewritten
            while(startPos >= target-sizeof(int)){

                safe_read(fdStart, &tmp, sizeof(int));
                startPos = lseek(fdStart, -2*(sizeof(int)), SEEK_CUR);

                safe_write(fdTarget, &tmp, sizeof(int));
                DupPos = lseek(fdTarget, -2*(sizeof(int)), SEEK_CUR);
            }
        }
        //write buffer to file
        lseek(fdStart, writePos*sizeof(int), SEEK_SET);
        int returnVal = safe_write(fdStart, buf, (*bufUsed)*sizeof(int));
        
        //Not needed, might be usefull for debug
        for(int j = 0; j < *bufUsed; j++){
            buf[j] = -1;
        }
        //Cleanup and exit
        *bufUsed = 0;

        //Note: threadArg->len is from initial start till the last point that data is part of bucket
        threadArg1->len += arg2PosRelative; 
        threadArg1->pos += arg2PosRelative;

        //dumpBinaryToText(fileInfo->name, "debug.txt");
        lseek(fdTarget, endPos, SEEK_SET);
        return returnVal/sizeof(int);
    }

    return 0;
}

void *ext_mergesort(void *arg){
    threadArg *fileInfo = (threadArg*)arg;

    //small enough split for data to be fit into mem
    if(fileInfo->len <= MAX_BUF_SIZE){
        
        //Sort data in buffer
        int *buf = malloc(MAX_BUF_SIZE*sizeof(int));
        int fd = open(fileInfo->name, O_RDWR);

        lseek(fd, (fileInfo->offset)*sizeof(int), SEEK_SET);
        safe_read(fd, buf, fileInfo->len*sizeof(int));

        qsort(buf, fileInfo->len, sizeof(int), compare);
    
        //Add sorted data to file
        //NOTE: expect fileInfo params to be handled by caller function
        lseek(fd, (fileInfo->offset)*sizeof(int), SEEK_SET);
        safe_write(fd, buf, fileInfo->len*sizeof(int));

        //return
        free(buf);
        close(fd);

        fileInfo->status = 1;
        return NULL;
    }
    //Create threads and wait for them to return:
    //allocate two args
    threadArg *threadArg1 = (threadArg*)malloc(sizeof(threadArg));
    threadArg *threadArg2 = (threadArg*)malloc(sizeof(threadArg));

    //initialize args for each thread
    threadArg1->name = fileInfo->name;
    threadArg1->len = (fileInfo->len)/2;
    threadArg1->offset = fileInfo->offset; //NOTE: offset should never be changed again
    threadArg1->pos = 0;
    threadArg1->status = 0;

    threadArg2->name = fileInfo->name;
    threadArg2->len = fileInfo->len - threadArg1->len;
    threadArg2->offset = fileInfo->offset + threadArg1->len; //might be +-1
    threadArg2->pos = 0;
    threadArg2->status = 0;

    //create threads
    pthread_t thread1;
    pthread_t thread2;

    pthread_create(&thread1, NULL, ext_mergesort, threadArg1);
    pthread_create(&thread2, NULL, ext_mergesort, threadArg2);

    //Wait while status is not updated
    while (threadArg1->status == 0 || threadArg2->status == 0) {
    
    }
    
    //Init buf
    int *buf = malloc(MAX_BUF_SIZE*sizeof(int));
    int bufUsed = 0;
    int numsWritten;

    //Open file
    int fd1 = open(fileInfo->name, O_RDWR);
    int fd2 = open(fileInfo->name, O_RDWR);

    //load bytes from offsets to chars
    //NOTE: offsets from beggining should have been already calculated: arg1->offset and arg2->offset
    int num1, num2;
    
    lseek(fd1, (threadArg1->offset)*sizeof(int), SEEK_SET);
    safe_read(fd1, &num1, sizeof(int));
    threadArg1->pos = 0;

    lseek(fd2, threadArg2->offset*sizeof(int), SEEK_SET);
    safe_read(fd2, &num2, sizeof(int));
    threadArg2->pos = 0;

    //main loop to write to file
    while (1){
        //Read every number from the first bucket
        if(threadArg1->pos == threadArg1->len-1){
            int dont_reenter=0; //Used to remember if last num from the first bucket is written
            
            //read and save every num from second bucket, and the last num from the first bucket
            while (threadArg2->pos != threadArg2->len-1) {
                
                //We have only one num remaining from the first bucket
                if(dont_reenter == 0 && num1 <= num2){
                    numsWritten = save(fd1, fd2, threadArg1, 
                            fileInfo, buf, &bufUsed, num1, 0);

                    fileInfo->pos += numsWritten;
                    dont_reenter++;
                }
                //Constantly reads and saves remaining nums from the second bucket
                numsWritten = save(fd1, fd2, threadArg1, 
                        fileInfo, buf, &bufUsed, num2, 0);
                
                threadArg2->pos++;
                fileInfo->pos += numsWritten;
                
                safe_read(fd2, &num2, sizeof(int));
            }

            //Save the last num from the second and (if not saved yet) the first bucket
            if(dont_reenter == 0 && num1 <= num2){
                save(fd1, fd2, threadArg1, 
                        fileInfo, buf, &bufUsed, num1, 0);
                fileInfo->pos += numsWritten;
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num2, 1);
                dont_reenter++;
            }
            else if(dont_reenter == 0){
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num2, 0);
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num1, 1);
            }
            else{
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num2, 1);
            }
            
            break;
        }
        else if(threadArg2->pos == threadArg2->len-1){
            int dont_reenter=0; //Used to remember if last num from the second bucket is written
            
            //read and save every num from first bucket, and the last num from the second bucket
            while (threadArg1->pos != threadArg1->len-1) {

                //We have only one num remaining from the second bucket
                if(dont_reenter == 0 && num2 <= num1){
                    numsWritten = save(fd1, fd2, threadArg1,
                            fileInfo, buf, &bufUsed, num2, 0);

                    fileInfo->pos += numsWritten;

                    dont_reenter++;
                }
                //Constantly reads and saves remaining nums from the second bucket
                numsWritten = save(fd1, fd2, threadArg1, 
                        fileInfo, buf, &bufUsed, num1, 0);
                
                threadArg1->pos++;
                fileInfo->pos += numsWritten;                

                safe_read(fd1, &num1, sizeof(int));
            }

            //Save the last num from the first and (if not saved yet) the second bucket
            if(dont_reenter == 0 && num2 <= num1){
                save(fd1, fd2, threadArg1,
                        fileInfo, buf, &bufUsed, num2, 0);
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num1, 1);
                         
                fileInfo->pos += numsWritten;
                dont_reenter++;
            }
            else if(dont_reenter == 0){
                save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num1, 0);
                save(fd1, fd2, threadArg1,
                    fileInfo, buf, &bufUsed, num2, 1);
            }
            else{
               save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num1, 1); 
            }

            break;
        }

        //Compare and write the smallest number
        //Read new number from the bucket we wrote to
        if(num1 <= num2){
            numsWritten = save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num1, 0);
            
            threadArg1->pos++;
            fileInfo->pos += numsWritten;
            
            safe_read(fd1, &num1, sizeof(int));
        }
        else{
            numsWritten = save(fd1, fd2, threadArg1, 
                    fileInfo, buf, &bufUsed, num2, 0);
            
            threadArg2->pos++;
            fileInfo->pos += numsWritten;
            
            safe_read(fd2, &num2, sizeof(int));
        }
    }
    //cleanup
    free(threadArg1);
    free(threadArg2);
    free(buf);
    close(fd1);
    close(fd2);
    //return
    fileInfo->status = 1;
    return NULL;
}

int main(int argc, char *argv[]){
    if(argv[1] == NULL){
        return -1;
    }
    threadArg *initialThread = (threadArg*)malloc(sizeof(threadArg));
    pthread_t iniThread;

    srand(time(NULL));

    int fdTmp = open(argv[1], O_RDONLY);
    off_t fileSize = lseek(fdTmp, 0, SEEK_END);

    initialThread->name = argv[1];
    initialThread->len = fileSize/sizeof(int);
    initialThread->offset = 0;
    initialThread->pos = 0;
    initialThread->status = 0;

    dumpBinaryToText(argv[1], "filePreSorted.txt");

    pthread_create(&iniThread, NULL, ext_mergesort, initialThread);
    while (initialThread->status == 0) {
    
    }
    free(initialThread);

    dumpBinaryToText(argv[1], "filePostSorted.txt");

    //check if file is sorted
    int currentNum, previousNum;
    ssize_t bytesRead;

    lseek(fdTmp, 0, SEEK_SET);

    // Read the first integer from the file
    bytesRead = read(fdTmp, &previousNum, sizeof(int));
    if (bytesRead != sizeof(int)) {
        // If we can't read the first integer, either the file is empty or an error occurred
        printf("found nothing at pos0\n");
        close(fdTmp);
        return 1;  // An empty file is considered sorted
    }

    // Loop through the file and check if each number is greater or equal to the previous one
    // Note: this was meant to verify that the program works correctly
    while ((bytesRead = safe_read(fdTmp, &currentNum, sizeof(int))) == sizeof(int)) {
        if (currentNum < previousNum) {
            printf("The file has not been sorted properly\n");
            close(fdTmp);
            return 1;
        }
        previousNum = currentNum;
    }

    // If we reach the end of the file without finding an out-of-order number, the file is sorted
    close(fdTmp);

    return 0;
    
}