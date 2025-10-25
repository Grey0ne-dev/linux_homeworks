#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

struct args1 {
    long *chunk;     // packing args in struct
    int size;
    long result;
};

void verify(int check, const char *errmsg) {  // helper for error handling
    if (check != 0) {
        perror(errcmsg);
        exit(1);
    }
}

void* fillWithChaos(void* arg) {
    struct args1* a = (struct args1*)arg;
    for (int i = 0; i < a->size; ++i) {  // name speaks itself
        a->chunk[i] = rand() % 1000;
    }
    return NULL;
}

void* countSum(void* arg) {
    struct args1* a = (struct args1*)arg;
    long sum = 0;
    for (int i = 0; i < a->size; ++i) { // same 
        sum += a->chunk[i];
    }
    a->result = sum;
    return NULL;
}

double diff_timespec(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;  // I used VLA so we dont have c++ stuff
}

int main(int argc, char *argv[]) {   // 
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <numCount> <threadCount>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));

    const long numCount = atol(argv[1]);
    const int threadCount = atoi(argv[2]);

    if (threadCount <= 0 || numCount <= 0) {
        fprintf(stderr, "Invalid arguments.\n");
        exit(1);
    }

    long *array = (long*)malloc(sizeof(long) * numCount);
    if (!array) {
        perror("malloc");
        exit(1);
    }

    const int chunkSize = numCount / threadCount;
    const int remainder = numCount % threadCount;

    pthread_t threads[threadCount];
    struct args1 args[threadCount];

    struct timespec startAll, endAll, startFill, endFill, startSum, endSum;

    clock_gettime(CLOCK_MONOTONIC, &startAll);
	// fill array
    clock_gettime(CLOCK_MONOTONIC, &startFill);

    for (int i = 0; i < threadCount; ++i) {
        args[i].chunk = array + i * chunkSize;
        args[i].size = (i == threadCount - 1) ? chunkSize + remainder : chunkSize;
        verify(pthread_create(&threads[i], NULL, fillWithChaos, &args[i]), "pthread_create");
    }

    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &endFill);

    // count sum
    clock_gettime(CLOCK_MONOTONIC, &startSum);

    for (int i = 0; i < threadCount; ++i) {
        verify(pthread_create(&threads[i], NULL, countSum, &args[i]), "pthread_create");
    }

    long total = 0;
    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], NULL);
        total += args[i].result;
    }

    clock_gettime(CLOCK_MONOTONIC, &endSum);
    clock_gettime(CLOCK_MONOTONIC, &endAll);
// hell yeah!

    printf("Total sum: %ld\n", total);
    printf("Fill time: %.6f sec\n", diff_timespec(startFill, endFill));
    printf("Sum time:  %.6f sec\n", diff_timespec(startSum, endSum));
    printf("Main time: %.6f sec\n",diff_timespec(startAll, endAll) - diff_timespec(startFill, endFill) - diff_timespec(startSum, endSum));
    printf("Total work: %.6f sec\n", diff_timespec(startAll, endAll));
    free(array);
    return 0;
}

