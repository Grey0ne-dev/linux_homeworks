#include "paralel_scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static void* worker(void* arg) {
    paralel_scheduler* p = arg;

    while (1) {
        pthread_mutex_lock(&p->lock);

        while (p->qsize == 0 && !p->stop)
            pthread_cond_wait(&p->cond, &p->lock);

        if (p->stop && p->qsize == 0) {
            pthread_mutex_unlock(&p->lock);
            return NULL;
        }

        task_t t = p->queue[p->qfront];
        p->qfront = (p->qfront + 1) % p->qcap;
        p->qsize--;

        pthread_mutex_unlock(&p->lock);

        t();
    }
}

int ps_init(paralel_scheduler* p, int thread_count, int queue_capacity) {
    p->threads = malloc(sizeof(pthread_t) * thread_count);
    if (!p->threads) return -1;

    p->queue = malloc(sizeof(task_t) * queue_capacity);
    if (!p->queue) { free(p->threads); return -1; }

    p->thread_count = thread_count;
    p->qcap = queue_capacity;
    p->qsize = 0;
    p->qfront = 0;
    p->stop = 0;

    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->cond, NULL);

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&p->threads[i], NULL, worker, p) != 0) {
            ps_destroy(p);
            return -1;
        }
    }

    return 0;
}

void ps_submit(paralel_scheduler* p, task_t t) {
    pthread_mutex_lock(&p->lock);

    if (p->qsize == p->qcap) {
        int new_cap = p->qcap * 2;
        task_t* new_queue = realloc(p->queue, sizeof(task_t) * new_cap);
        if (!new_queue) {
            pthread_mutex_unlock(&p->lock);
            return;
        }

        if (p->qfront > 0) {
            for (int i = 0; i < p->qsize; i++) {
                new_queue[i] = p->queue[(p->qfront + i) % p->qcap];
            }
            p->qfront = 0;
        }

        p->queue = new_queue;
        p->qcap = new_cap;
    }

    int pos = (p->qfront + p->qsize) % p->qcap;
    p->queue[pos] = t;
    p->qsize++;

    pthread_cond_signal(&p->cond);
    pthread_mutex_unlock(&p->lock);
}

void ps_destroy(paralel_scheduler* p) {
    pthread_mutex_lock(&p->lock);
    p->stop = 1;
    pthread_cond_broadcast(&p->cond);
    pthread_mutex_unlock(&p->lock);

    for (size_t i = 0; i < p->thread_count; i++)
        pthread_join(p->threads[i], NULL);

    pthread_mutex_destroy(&p->lock);
    pthread_cond_destroy(&p->cond);

    free(p->threads);
    free(p->queue);
}
