#include <stdio.h> 
#include <stdlib.h> 
#include "queue.h"


void CreateQueue(queue *Q, int size)
{
  int i;
  *Q = malloc (sizeof(Queue));
  (*Q)->begin = 0;
  (*Q)->end = 0;
  (*Q)->elemCount = size;
  (*Q)->size = size;
  (*Q)->q = (int *) malloc (sizeof(int) * size);
  for (i = 0; i < size; i++) 
    (*Q)->q[i] = i + 1;
}

/* Resource allocation scheduling */
void UpdateQueue(queue *Q, int size)
{
  int i;
  (*Q)->begin = 0;
  (*Q)->end = 0;
  (*Q)->elemCount = size;
  (*Q)->size = size;
  (*Q)->q = (int *) realloc ((*Q)->q, sizeof(int) * size);
  for (i = 0; i < size; i++) 
    (*Q)->q[i] = i + 1;
}

void DestroyQueue (queue *Q)
{
  free((*Q)->q); 
  free(*Q);
}


void Dump(queue *Q) {
  int i;
  int size = (*Q)->size;
  int begin = (*Q)->begin;
  int end = (*Q)->end;

  for (i = 0; i < size; i++) 
    printf("%2d ", (*Q)->q[i]);

  printf("\n");

  for (i = 0; i < size; i++) 
    if (i == begin && i == end) printf(" o ");
    else if (i == begin) printf(" b ");
    else if (i == end) printf(" e ");
    else printf("   ");

  printf("\n");
}

int Enqueue(queue *Q, int elem) {

  int size = (*Q)->size;
  int begin = (*Q)->begin;
  int end = (*Q)->end;

  if (begin == end && (*Q)->elemCount != 0) {
    //printf("Debug: Q full\n");
    return -1;
  }

  //printf("Debug: Enqueue %d at %d\n", elem, end);
  (*Q)->q[end++] = elem;
  (*Q)->elemCount++;
  (*Q)->end = end % size;

  //Dump(Q);
}

int Dequeue(queue *Q) {

  if (Empty(Q)) return -1;

  int size = (*Q)->size;
  int begin = (*Q)->begin;
  int end = (*Q)->end;
  int elem = (*Q)->q[begin];

  //printf("Debug: Dequeue %d at %d\n", elem, begin);

  (*Q)->q[begin++] = -1;
  (*Q)->begin = begin %  size;
  (*Q)->elemCount--;

  //Dump(Q);
  return elem;
}

int Empty(queue *Q) {
  if ((*Q)->elemCount == 0) {
    //printf("Debug: Q empty\n");
    return 1;
  }
  return 0;
}

