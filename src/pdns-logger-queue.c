
#include <pthread.h>
#include "pdns-logger.h"

typedef struct fifo_item_s fifo_item_t;
struct fifo_item_s {
    void *data;
    fifo_item_t *next;
};

struct fifo_s {
    pthread_mutex_t lock;
    fifo_item_t *head;
    fifo_item_t *tail;
};

fifo_t *fifo_init(void) {
    fifo_t *fifo = NULL;
    pthread_mutexattr_t attr;

    fifo = malloc(sizeof(fifo_t));
    if ( fifo != NULL ) {
        memset(fifo, 0, sizeof(fifo_t));
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&fifo->lock, &attr);
    }

    return fifo;
}

pdns_status_t fifo_push(fifo_t *fifo, void *value) {
    fifo_item_t *item;
    if ( fifo == NULL || value == NULL ) {
        return PDNS_NO;
    }

    item = malloc(sizeof(fifo_item_t));
    if ( item != NULL ) {
        item->data = value;

        pthread_mutex_lock(&fifo->lock);
        if ( fifo->tail == NULL && fifo->head == NULL ) {
            fifo->head = fifo->tail = item;
        }
        else if ( fifo->tail != NULL || fifo->head != NULL ) {
            /* Should not happen */
            assert(0);
        }
        else {
            item->next = fifo->head;
            fifo->head = item;
        }
        pthread_mutex_unlock(&fifo->lock);

        return PDNS_OK;
    }

    safe_free(item);

    return PDNS_NO;
}

void *fifo_pop(fifo_t *fifo) {
    void *ret;
    fifo_item_t *item = NULL;

    if ( fifo == NULL ) {
        return NULL;
    }

    pthread_mutex_lock(&fifo->lock);
    if ( fifo->head == NULL && fifo->tail == NULL ) {
        /* Empty */
    }
    else if ( fifo->head != NULL && fifo->tail == fifo->head ) {
        item = fifo->head;
        fifo->head = fifo->tail = NULL;
    }
    else {
        item = fifo->head;
        fifo->head = item->next;
    }
    pthread_mutex_unlock(&fifo->lock);

    if ( item != NULL ) {
        ret = item->data;
        safe_free(item);
        return ret;
    }

    return NULL;
}
