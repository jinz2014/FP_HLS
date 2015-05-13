#ifndef QUEUE_
#define QUEUE_
typedef struct Queue {
  int begin;
  int end;
  int elemCount;
  int *q;
  int size;
} Queue, *queue;

void CreateQueue (queue *Q, int size);
void UpdateQueue(queue *Q, int size);
void DestroyQueue (queue *Q);
int Enqueue(queue *Q, int elem);
int Dequeue(queue *Q);
int Empty(queue *Q);
void Dump(queue *Q);

#endif
