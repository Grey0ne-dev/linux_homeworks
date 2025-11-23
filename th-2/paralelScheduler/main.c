#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "paralel_scheduler.h"

// Example tasks
void task1() {
    printf("Task 1 started\n");
    sleep(1);
    printf("Task 1: Some useful work\n");
    sleep(1);
    printf("Task 1 finished\n");
}

void task2() {
    printf("Task 2 started\n");
    sleep(2);
    printf("task 2: Some useful work\n");
    sleep(4);
    printf("Task 2 finished\n");
}

void task3() {
    printf("Task 3 started\n");
    sleep(1);
    printf("Task 3: Some Useful workn\n");
    sleep(2);
    printf("Task 3 finished\n");
}

int main() {
    paralel_scheduler scheduler;

    
    if (ps_init(&scheduler, 3, 10) != 0) {
        fprintf(stderr, "Failed to initialize scheduler\n"); // senc em anum vor returnic brnem, classi mej normal obrabotka chem sarqel
        return 1;
    }

    ps_submit(&scheduler, task1);
    ps_submit(&scheduler, task2);
    ps_submit(&scheduler, task3);

    sleep(10); //jamanak ranq vor ashxaten

    ps_destroy(&scheduler);

    printf("All tasks completed.\n");
    return 0;
}
