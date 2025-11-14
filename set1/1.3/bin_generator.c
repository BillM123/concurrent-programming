#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_INTEGERS 10000  // Number of random integers to generate
#define MAX_NUM NUM_INTEGERS
#define MIN_NUM 0

int generateRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

int main() {
    const char *filename = "data.bin";
    int randomNumber;
    
    // Seed the random number generator
    srand(time(NULL));

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Write random integers to the binary file
    for (size_t i = 0; i < NUM_INTEGERS; i++) {
        randomNumber = generateRandomNumber(MIN_NUM, MAX_NUM);  // Generate a random integer
        if (fwrite(&randomNumber, sizeof(int), 1, file) != 1) {
            perror("Error writing to file");
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    //printf("Successfully wrote %d random integers to %s\n", NUM_INTEGERS, filename);

    fclose(file);
    return EXIT_SUCCESS;
}
