#include "queue.h"
//-----------------------------------------------
// Test a 4-entry queue
//-----------------------------------------------
int main()
{
  int elem = 0;
  queue *Q;
  CreateQueue(Q, 4);
  
  Dequeue(Q);
  Enqueue(Q, ++elem);
  Enqueue(Q, ++elem);
  Enqueue(Q, ++elem);
  Enqueue(Q, ++elem);
  Dequeue(Q);
  Enqueue(Q, ++elem);
  Enqueue(Q, ++elem);
  Dequeue(Q);
  Enqueue(Q, ++elem);
  Dequeue(Q);
  Enqueue(Q, ++elem);
  Dequeue(Q);
  Dequeue(Q);
  Dequeue(Q);
  Dequeue(Q);

  DestroyQueue(Q);

  return 0;
}


