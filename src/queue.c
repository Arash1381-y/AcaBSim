#include "queue.h"

struct queue
{
  const void **items;
  unsigned int size;
  unsigned capacity;
};

queue_t *
queue_init (unsigned capacity)
{
  queue_t *queue = malloc (sizeof (queue_t));

  if (!queue)
    return NULL;

  queue->size = 0;
  queue->capacity = capacity;
  queue->items = malloc (capacity * sizeof (const void *));

  if (!queue->items)
  {
    free (queue);
    return NULL;
  }

  return queue;
}

size_t
queue_size (queue_t *queue)
{
  if (!queue)
    return 0;

  return queue->size;
}

int
queue_push (queue_t *queue, const void *item)
{
  if (!queue || !item)
    return 0;

  if (queue->size >= queue->capacity)
  {
    if (!queue_resize (queue, queue->capacity * 2))
      return 0;
  }
  else if (queue_is_empty (queue))
  {
    queue->size = 1;
    queue->items[0] = item;

    return 1;
  }

  const struct tile *last = queue->items[0];
  for (unsigned int i = 1; i <= queue->size; i++)
  {
    const struct tile *next = queue->items[i];
    queue->items[i] = last;
    last = next;
  }

  queue->items[0] = item;
  queue->size += 1;

  return 1;
}

int
queue_is_empty (queue_t *queue)
{
  return queue->size == 0;
}

const void *
queue_top (queue_t *queue)
{
  if (!queue_is_empty (queue))
    return queue->items[queue->size - 1];
  else
    return NULL;
}

int
queue_pop (queue_t *queue)
{
  if (queue_is_empty (queue))
    return 0;

  queue->size -= 1;
  queue->items[queue->size] = NULL; // not necessary

  if (queue->size < queue->capacity / 2)
    queue_resize (queue, queue->capacity / 2);

  return 1;
}

void
queue_free (queue_t *queue)
{
  free (queue->items);
  free (queue);
}

int
queue_resize (queue_t *queue, unsigned capacity)
{
  if (!queue)
    return 0;

  const void **items = realloc (queue->items, capacity * sizeof (const void *));

  if (!items)
    return 0;

  queue->capacity = capacity;
  queue->items = items;

  if (queue->size > queue->capacity)
    queue->size = queue->capacity;

  return 1;
}

// Create an iterator for the queue
queue_iterator_t *
queue_iterator_create (queue_t *queue)
{
  if (!queue || queue_is_empty (queue))
    return NULL;

  queue_iterator_t *iterator = malloc (sizeof (queue_iterator_t));
  if (!iterator)
    return NULL;

  iterator->queue = queue;
  iterator->index = 0; // Start from the front of the queue

  return iterator;
}

// Check if the iterator has a next element
int
queue_iterator_has_next (queue_iterator_t *iterator)
{
  if (!iterator)
    return 0;

  return iterator->index < iterator->queue->size;
}

// Get the next item from the queue
const void *
queue_iterator_next (queue_iterator_t *iterator)
{
  if (!iterator || !queue_iterator_has_next (iterator))
    return NULL;

  const void *item = iterator->queue->items[iterator->index];
  iterator->index++;

  return item;
}

// Destroy the iterator
void
queue_iterator_destroy (queue_iterator_t *iterator)
{
  if (iterator)
  {
    free (iterator);
  }
}
