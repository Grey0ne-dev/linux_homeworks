#ifndef PARALEL_SCHEDULER_H
#define PARALEL_SCHEDULER_H

#include <pthread.h>
#include <stddef.h>

typedef void (*task_t)(void);

typedef struct {
    pthread_t* threads;
    size_t thread_count;

    task_t* queue;
    int qsize;  
    int qcap;
    int qfront;

    pthread_mutex_t lock;
    pthread_cond_t cond;

    int stop;                // stop flag
} paralel_scheduler;

int ps_init(paralel_scheduler* p, int thread_count, int queue_capacity);


void ps_submit(paralel_scheduler* p, task_t t);

void ps_destroy(paralel_scheduler* p);

#endif

