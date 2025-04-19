#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Global variables for matrices and size
int N;
int **A, **B, **C;
int numThreads;

// Worker thread function for matrix multiplication where each thread computes a portion of the result matrix C
void *worker(void *arg) {
    int threadId = *((int *)arg);
    
    // Calculating the range of rows for this thread
    int rowsPerThread = N / numThreads;
    int startRow = threadId * rowsPerThread;
    int endRow;
    
    // Handling the case where N is not perfectly divisible by numThreads
    if (threadId == numThreads - 1) {
        endRow = N; // Last thread takes any remaining rows
    } else {
        endRow = startRow + rowsPerThread;
    }
    
    // Performing matrix multiplication for the assigned rows
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return NULL;
}

// Reads the integers from matrix A and B from their respective files
int readMatrix(char *filename, int **matrix) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        return 1;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fscanf(fp, "%d", &matrix[i][j]) != 1) {
                printf("Error reading matrix from %s\n", filename);
                fclose(fp);
                return 1;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

// Writing the output to matrix C
int writeMatrix(char *filename, int **matrix) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Cannot open file %s for writing\n", filename);
        return 1;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(fp, "%d\n", matrix[i][j]);
        }
    }
    
    fclose(fp);
    return 0;
}

// Main function
int main(int argc, char *argv[]) {
    // Checking command line args for validity. Exit in error
    if (argc != 6) {
        printf("Usage: %s <numThreads> <N> <filename for A> <filename for B> <filename for C>\n", argv[0]);
        return 1;
    }

    numThreads = atoi(argv[1]); // Converts the numeric string into an integer value
    N = atoi(argv[2]); // Size 

   
    char *fileA = argv[3]; // Name of File A
    char *fileB = argv[4]; // Name of File B
    char *fileC = argv[5]; // Name of the file you want to write the output to -> Let's call it File C

    /* Argument Checking for size of matrix and the number of threads*/
    if (N <= 0) {
        printf("Invalid input, please enter a valid matrix size!");
        return 1;
    }
    if (numThreads <= 0) {
        printf("Invalid input, number of threads must be positive");
        return 1;
    }

    // Allocating memory for matrices A, B, and C
    A = (int **)malloc(N * sizeof(int *));
    B = (int **)malloc(N * sizeof(int *));
    C = (int **)malloc(N * sizeof(int *));

    if (A == NULL || B == NULL || C == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        A[i] = (int *)malloc(N * sizeof(int));
        B[i] = (int *)malloc(N * sizeof(int));
        C[i] = (int *)malloc(N * sizeof(int));
        
        if (A[i] == NULL || B[i] == NULL || C[i] == NULL) {
            printf("Memory allocation failed\n");
            return 1;
        }
    }
 
    // Calling the function readMatrix() for fileA and fileB and cleaning garbage memory 
    if (readMatrix(fileA, A) != 0) {
        // Clean up allocated memory before exiting
        for (int i = 0; i < N; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(C);
        return 1;
    }
    if (readMatrix(fileB, B) != 0) {
        // Clean up allocated memory before exiting
        for (int i = 0; i < N; i++) {
            free(A[i]);
            free(B[i]);
            free(C[i]);
        }
        free(A);
        free(B);
        free(C);
        return 1;
    }

    // Main thread starts the timer
    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Creating worker threads
    pthread_t *threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
    int *threadIds = (int *)malloc(numThreads * sizeof(int));

    if (threads == NULL || threadIds == NULL) {
        printf("Memory allocation for threads failed\n");
        return 1;
    }

    for (int i = 0; i < numThreads; i++) {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, worker, (void *)&threadIds[i]);
    }

    // Waiting for threads to terminate
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Stoping timer and calculating the execution time
    gettimeofday(&end, NULL);
    double execTime = (end.tv_sec - start.tv_sec) + 
                    ((end.tv_usec - start.tv_usec) / 1000000.0);

    // Printing execution time
    printf("Execution time: %f seconds\n", execTime);

    if (writeMatrix(fileC, C) != 0) {
        printf("Error writing to output file\n");
    }

    // Free allocated memory
    for (int i = 0; i < N; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }
    free(A);
    free(B);
    free(C);
    free(threads);
    free(threadIds);


    return 0;
}
