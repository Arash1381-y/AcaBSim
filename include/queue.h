#ifndef _QUEUE_H
#define _QUEUE_H

#define QUEUE_BASE_SIZE 128

#include <stdlib.h>

/**
 * @file queue.h
 * @brief queue is a simple library for basic dynamic queue in C. It contains
 * all the primitive for a queue.
 */

/**
 * A structure which represent a generic queue
 */
struct queue;
typedef struct queue queue_t;

/**
 * Returns an new empty queue with an initial capacity
 * Return NULL on failure
 */
queue_t *queue_init (unsigned capacity);

/**
 *  Returns the size of the queue
 */
size_t queue_size (queue_t *queue);

/**
 *  Predicates if the queue is empty or not
 */
int queue_is_empty (queue_t *queue);

/**
 * Returns a pointer to the element on the top of a queue
 * Returns NULL on error or if it's empty
 */
const void *queue_top (queue_t *queue);

/**
 * Pushes in an element inside (at the end of) a queue
 * Returns true or false wether the operation was successful
 */
int queue_push (queue_t *queue, const void *item);

/**
 * Pops out the element on the top of a queue
 * Returns true or false wether the operation was successful
 */
int queue_pop (queue_t *queue);

/**
 * Frees all the ressources associated to the queue
 */
void queue_free (queue_t *queue);

/**
 * Resizes the capacity of the queue
 * Returns true or false wether the operation was successful
 */
int queue_resize (queue_t *queue, unsigned capacity);

#endif //_QUEUE_H_