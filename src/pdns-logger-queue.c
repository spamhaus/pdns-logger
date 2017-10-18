/*
 * Powerdns logger daemon
 * ----------------------
 *
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (C) 2017, Spamhaus Technology Ltd, London
 *
 * The Initial developer of the Original code is:
 * Massimo Cetra
 *
 */

#include <sys/time.h>
#include <pthread.h>
#include "pdns-logger.h"

typedef struct fifo_item_s fifo_item_t;
struct fifo_item_s {
    void *data;
    fifo_item_t *next;
};

struct fifo_s {
    pthread_mutex_t lock;

    pthread_mutex_t sync_mutex;
    pthread_cond_t sync_cond;

    fifo_item_t *head;
    fifo_item_t *tail;
};

static void fifo_wait_signal(fifo_t * fifo) {
    int msw = 1000;
    struct timeval tv;
    struct timespec ts;

    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + msw / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (msw % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

    pthread_mutex_lock(&fifo->sync_mutex);
    pthread_cond_timedwait(&fifo->sync_cond, &fifo->sync_mutex, &ts);
    pthread_mutex_unlock(&fifo->sync_mutex);

    return;
}

static void fifo_send_signal(fifo_t * fifo) {
    pthread_mutex_lock(&fifo->sync_mutex);
    pthread_cond_signal(&fifo->sync_cond);
    pthread_mutex_unlock(&fifo->sync_mutex);
    return;
}

fifo_t *fifo_init(void) {
    fifo_t *fifo = NULL;
    pthread_mutexattr_t attr;

    fifo = malloc(sizeof(fifo_t));
    if (fifo != NULL) {
        memset(fifo, 0, sizeof(fifo_t));
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&fifo->lock, &attr);

        pthread_mutex_init(&fifo->sync_mutex, NULL);
        pthread_cond_init(&fifo->sync_cond, NULL);

    }

    return fifo;
}

static pdns_status_t fifo_push(fifo_t * fifo, void *value) {
    fifo_item_t *item;
    if (fifo == NULL || value == NULL) {
        return PDNS_NO;
    }

    item = malloc(sizeof(fifo_item_t));
    if (item != NULL) {
        item->data = value;

        pthread_mutex_lock(&fifo->lock);
        if (fifo->tail == NULL && fifo->head == NULL) {
            fifo->head = fifo->tail = item;
        } else if (fifo->tail != NULL || fifo->head != NULL) {
            /* Should not happen */
            assert(0);
        } else {
            item->next = fifo->head;
            fifo->head = item;
        }
        pthread_mutex_unlock(&fifo->lock);

        return PDNS_OK;
    }

    safe_free(item);

    return PDNS_NO;
}

static void *fifo_pop(fifo_t * fifo) {
    void *ret;
    fifo_item_t *item = NULL;

    if (fifo == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&fifo->lock);
    if (fifo->head == NULL && fifo->tail == NULL) {
        /* Empty */
    } else if (fifo->head != NULL && fifo->tail == fifo->head) {
        item = fifo->head;
        fifo->head = fifo->tail = NULL;
    } else {
        item = fifo->head;
        fifo->head = item->next;
    }
    pthread_mutex_unlock(&fifo->lock);

    if (item != NULL) {
        ret = item->data;
        safe_free(item);
        return ret;
    }

    return NULL;
}

pdns_status_t fifo_push_item(fifo_t * fifo, void *data) {

    if (fifo == NULL || data == NULL) {
        return PDNS_NO;
    }

    if (fifo_push(fifo, data) == PDNS_OK) {
        fifo_send_signal(fifo);
        return PDNS_OK;
    }

    return PDNS_NO;
}

void *fifo_pop_item(fifo_t * fifo) {
    if (fifo == NULL) {
        return NULL;
    }

    fifo_wait_signal(fifo);
    return fifo_pop(fifo);
}

void fifo_lock(fifo_t * fifo) {
    if (fifo != NULL) {
        pthread_mutex_lock(&fifo->lock);
    }
}

void fifo_unlock(fifo_t * fifo) {
    if (fifo != NULL) {
        pthread_mutex_unlock(&fifo->lock);
    }
}
