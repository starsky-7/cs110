#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

Queue *queue_create(void)
{
  Queue *queue = malloc(sizeof(Queue));
// If the space to be malloc are not enough, throw error
	if (queue == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
}

  queue->size = 0;
  queue->capacity = QUEUE_INITIAL_CAPACITY;
  queue->data = malloc(sizeof(double) * queue->capacity);

// If the space to be malloc are not enough, throw error
	if (queue == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		free(queue); // Free the space that allcated to queue before
		exit(EXIT_FAILURE);
}
		
  return queue;
}

void push(Queue *queue, double element)
{
// Check whether the queue passed to func is valid
  if (queue == NULL) {
	fprintf(stderr, "Error: invalid queue.\n");
	exit(EXIT_FAILURE);
  }

  if (queue->size == queue->capacity)
  {
    int capacity = queue->capacity * 2;
    queue->data = realloc(queue->data, sizeof(double) * capacity);
// If the space to be malloced are not enough, throw error, and maintain the queue's structure
	if (queue->data == NULL) {
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
  }

    queue->capacity = capacity;
  }

  int insert_index = queue->size % queue->capacity;
  queue->data[insert_index] = element;
  queue->size++;
}

double back(Queue *queue)
{
// Check whether the queue passed to func is valid
  if (queue == NULL) {
    fprintf(stderr, "Error: invalid queue.\n");
    exit(EXIT_FAILURE);
  }
  
// If back() accesses an empty queue, throw an error 
  if (queue->size == 0)
  {
    fprintf(stderr, "Error: queue is empty.\n");
    exit(EXIT_FAILURE);
  }

// If the queue's data doesn't exists, throw an error   
  if (queue->data == NULL) {
    fprintf(stderr, "Error: data doesn't exist.\n");
    exit(EXIT_FAILURE);
  }  

  return queue->data[queue->size - 1];
}

void queue_free(Queue *queue)
{
// Check whether the queue passed to func is valid
  if (queue == NULL) {
    fprintf(stderr, "Error: invalid queue.\n");
    exit(EXIT_FAILURE);
  }
  free(queue->data);
  free(queue);
}

void another_queue_free(Queue **queue)
{
  // Check whether the queue passed to func is valid
  if (queue == NULL || *queue == NULL) {
    fprintf(stderr, "Error: invalid queue.\n");
    exit(EXIT_FAILURE);
  }

  // free data
  free((*queue)->data);

  // free queue itself
  free(*queue);

  // set *queue to NULL
  *queue = NULL;
}
