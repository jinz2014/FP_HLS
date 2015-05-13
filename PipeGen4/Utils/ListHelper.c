#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "Schedule.h"

#define DEBUG


/* List Insert based on different priority (e.g.  operation path length, mobility) */
List ListInsert(List head, int nodeID, int priority, enum priority priority_type) {
  List node;
  List p0, p1, p2;
  int condition;

  p0 = p2 = head;

  /* Make sure no repetition of any operation node */
  while (p2 != NULL) {
    if (p2->nodeID == nodeID)
      return head;
    else
      p2 = p2->next;
  }

  node = malloc(sizeof(struct OpAvailable));
  node->nodeID = nodeID;
  node->priority = priority;
  //printf("Inserting node... %d \n", nodeID);

  if (head == NULL) {
    head = node;
    node->next = NULL;
  } else {
    while (p0 != NULL) {
      switch (priority_type) {
        case LENGTH: 
          /* priority is path length */
          condition = (priority > p0->priority);
          break;
        case MOBILITY: 
          /* priority is mobility */
          condition = (priority < p0->priority);
          break;
        case NONE: 
          /* priority is mobility */
          condition = 0;
          break;
        default:
          condition = 0;
          printf("Warnings: What is the priority type?");
          exit(1);
      }

      if (condition) { 
        if (p0 == head) {
          node->next = head;
          head = node;
        } else {
          node->next = p0;
          p1->next = node;
        }
        return head;
      }

      p1 = p0;
      p0 = p0->next;
    }

    // Append the node at the end of the list.
    //(i.e. topological order when two operations
    // have the same mobility)
    //printf("Append node %d \n", nodeID);
    p1->next = node;
    node->next = NULL;
 }   
 return (head);
}

//----------------------------------------------------------------------------
// Delete the node with the given operation number 
//----------------------------------------------------------------------------
List ListDelete(List head, int nodeID) {
  assert (head != NULL);

  List p1, p2;
  p1 = head;

  while (nodeID != p1->nodeID && p1->next != NULL) {
    p2 = p1;
    p1 = p1->next;
  }

  if (nodeID == p1->nodeID) {

    if (p1 == head)
      head = p1->next;
    else
      p2->next = p1->next;

    free(p1);

  } else {

    printf("Node ID %d not found\n", nodeID);
    exit(1);
  }
  return (head);
}

  
void ListPrint(List head) {
  List p = head;
  printf("--------List Nodes--------\n");
  while (p != NULL) {
    printf("%3d ", p->nodeID);
    p = p->next;
  }
  printf("\n");
  p = head;
  while (p != NULL) {
    printf("%3d ", p->priority);
    p = p->next;
  }
  printf("\n--------------------------\n");
}

/* Calculate the operation path length from nodeID to SINK */
/* Incorrect if node's outdegree > 1
int PathLength(int nodeID) {
  int length = 0;
  int i = nodeID;
  int j = nodeID;
  while(1) {
    length += (DFG[i]->opLatency + 1);
    //if (DFG[i]->opDestID == SINK) 
    if (DFG[i]->opDest[0] == SINK) 
      break;
    i = DFG[i]->opDestID;
  }
  #ifdef DEBUG
  printf("Path length from node %d to SINK is %d\n", j, length);
  #endif
  return length;
}
*/

  
void SetMobility(int *ASAP_slots, int *ALAP_slots, int maxInputPortNu) {

  int i;
  int mobility;

  printf("\n-------- Mobility distribution -------\n");

  for (i = 0; i < NODE_NU; i++) {
    if (DFG[i]->opSrc[0] >= 0) {
      if (ALAP_slots[i] < ASAP_slots[i]) 
        printf("Error: ALAP_slots[%d] < ASAP_slots[%d]", i, i);
      mobility = ALAP_slots[i] - ASAP_slots[i] + 1;
      DFG[i]->opPriority = mobility;
      printf("Node %3d : <%3d, %3d> = %3d\n", i, ASAP_slots[i], ALAP_slots[i], mobility); 
    }
  }

  #ifdef DEBUG
  printf("\n");
  #endif
}

