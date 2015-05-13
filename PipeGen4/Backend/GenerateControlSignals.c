#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Schedule.h"

// compNu can be shared register number or MUX number
FSM_outputs_ptr *CreateCntEnableTable(int compNu) {

  int i;

  FSM_outputs_ptr *CntEnableTable = 
    (FSM_outputs_ptr *) malloc (sizeof(FSM_outputs_ptr) * compNu);

  for (i = 0; i < compNu; i++) {
    CntEnableTable[i] = (FSM_outputs_ptr) malloc (sizeof(FSM_outputs));
    CntEnableTable[i]->cntNu = 0;
    CntEnableTable[i]->enableTime = NULL;
  }
  return CntEnableTable; 
}

void FreeCntEnableTable
(FSM_outputs_ptr *CntEnableTable, int compNu) {

  int i;
  for (i = 0; i < compNu; i++) {
    if (CntEnableTable[i] != NULL) {
      free(CntEnableTable[i]->enableTime);
      free(CntEnableTable[i]);
    }
  }
  free(CntEnableTable);
}

// 
FSM_outputs_ptr *UpdateCntEnableTable 
(FSM_outputs_ptr *CntEnableTable, int compNu, int multiple, int latency) {

  int *enableTimePtr;
  int cntNu = ++(CntEnableTable[compNu]->cntNu);

  enableTimePtr = (CntEnableTable[compNu]->enableTime == NULL) ? 
    NULL : CntEnableTable[compNu]->enableTime;

  CntEnableTable[compNu]->enableTime = 
    (int *) realloc (enableTimePtr, sizeof(int) * cntNu);

  CntEnableTable[compNu]->enableTime[cntNu - 1] = multiple * latency;

  return CntEnableTable; 
}

/* Collect all the MUXs in an increasing order of MUX select enable time */
void CreateMuxSelTable(int MuxCnt) {
  int i;
  assert(MuxCnt != 0);
  MuxSelTable = (MuxSelList *) malloc (sizeof(MuxSelList) * MuxCnt);
  for (i = 0; i < MuxCnt; i++) 
    MuxSelTable[i] = NULL;
}

void FreeMuxSelTable(int MuxCnt) {
  int muxNu;
  MuxSelList head, p, p1, p2;

  for (muxNu = 0; muxNu < MuxCnt; muxNu++) {
    p = head = MuxSelTable[muxNu];
    // For each loop, go to the last node and delete it
    while (p != NULL) {
      p2 = p;
      p1 = p->next;
      if (p1 != NULL) {
        while (p1->next != NULL) {
          p2 = p1;
          p1 = p1->next;
        }
        p2->next = p1->next;
        free(p1);
        p = head;
      } else {
        free(p);
        break; 
      }
    }
  }
  free(MuxSelTable);
}

void PrintMuxSelList(int *MuxCnt) {
  int i;
  MuxSelList head, p0;

  for (i = 0; i < *MuxCnt; i++) {
    p0 = head = MuxSelTable[i];
    myprintf("Mux ID %d:\n", i + 1);
    while (p0 != NULL) {
      myprintf("startTime %d selVal %d inputNu %d\n", 
          p0->startTime, p0->selVal, p0->inputNu);
      p0 = p0 -> next;
    }
    myprintf("\n");
  }
}

void MuxSelListInsert(int muxNu, int startTime, int selVal, int inputNu) {
  MuxSelList mux;
  MuxSelList p0, p1, head;
  int key;

  // No insertion if Mux select value is 0
  if (selVal == 0) return;

  p0 = head = MuxSelTable[muxNu];

  if (head == NULL) {

    myprintf("Adding mux start time and selVal at MuxSelTable[%d]\n", muxNu);
    mux = (MuxSelList) malloc(sizeof(struct muxSelTime));
    mux->startTime = startTime;
    mux->selVal = selVal;
    mux->inputNu = inputNu;
    mux->addMuxReg = 0; // 9/1/2012

    head = mux;
    mux->next = NULL;

  } else {

    // No repetition
    while (p0 != NULL) {
      if (p0->startTime == startTime && 
          p0->selVal == selVal && 
          p0->inputNu == inputNu)
        return;
      else
        p0 = p0->next;
    }

    myprintf("Adding mux start time and selVal at MuxSelTable[%d]\n", muxNu);
    mux = (MuxSelList) malloc(sizeof(struct muxSelTime));
    mux->startTime = startTime;
    mux->selVal = selVal;
    mux->inputNu = inputNu;
    mux->addMuxReg = 0; // 9/1/2012

    p0 = head;
    while (p0 != NULL) {

      // sorts the Mux sel time in ascending order
      key = (startTime < p0->startTime) ? 1 : 0;

      if (key) {
        if (p0 == head) {
          mux->next = head;
          head = mux;
        } else {
          mux->next = p0;
          p1->next = mux;
        }
        MuxSelTable[muxNu] = head;
        return;
      }

      p1 = p0;
      p0 = p0->next; 
    }

    // Append the node at the end of the list
    p1->next = mux;
    mux->next = NULL;
  }
  MuxSelTable[muxNu] = head;
}


/* Collect all the FUs in an increasing enable time order */

/*
void CreateFuGoTable() {
  enum operation op_type;
  int i;
  int size;
  int fu_nu;

  FuGoTable = (FuGoList**) malloc (sizeof(FuGoList*) * OP_NU);

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    // Number of non-port FUs
    size = (*RQ[op_type])->size;
    FuGoTable[op_type] = (FuGoList *) malloc (sizeof(FuGoList) * size);
    for (fu_nu = 0; fu_nu < size; fu_nu++)
      FuGoTable[op_type][fu_nu] = NULL;
  }
}

void FreeFuGoTable() {
  enum operation op_type;
  int i;
  int size;
  int fu_nu;
  FuGoList head, p, p1, p2;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    for (fuNu = 0; fuNu < size; fuNu++) {
      p = head = FuGoTable[fuNu];
      // For each loop, go to the last node and delete it
      while (p != NULL) {
        p2 = p;
        p1 = p->next;
        if (p1 != NULL) {
          while (p1->next != NULL) {
            p2 = p1;
            p1 = p1->next;
          }
          p2->next = p1->next;
          free(p1);
          p = head;
        } else {
          free(p);
          break; 
        }
      }
    }
    free(FuGoTable[op_type]);
  }

  free(FuGoTable);
}

void PrintFuGoList(FILE *vfp) {
  enum operation op_type;
  int i;
  int size;
  int fu_nu;
  FuGoList p;

  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    fprintf(vfp, "FU type %d: ", op_type);
    for (fuNu = 0; fuNu < size; fuNu++) {
      p = FuGoTable[op_type][fuNu];
      if (p != NULL)
        fprintf(vfp, "number %d:\n", fu_NU);
      while (p != NULL) {
        fprintf(vfp, "startTime %d \n", p->startTime);
        p = p -> next;
      }
    }
    fprintf(vfp, "\n");
  }
}

void FuGoListInsert(enum operation opType, int fuNu, int startTime) {
  FuGoList fu;
  FuGoList p0, p1, head;
  int key;

  p0 = head = FuGoTable[optype][fuNu];

  fu = (FuGoList) malloc(sizeof(struct fuGoTime));

  fu->startTime = startTime;
  
  if (head == NULL) {

    head = fu;
    fu->next = NULL;

  } else {

    while (p0 != NULL) {

      // sorts the Mux sel time in ascending order
      key = (startTime < p0->startTime) ? 1 : 0;

      if (key) {
        if (p0 == head) {
          fu->next = head;
          head = fu;
        } else {
          fu->next = p0;
          p1->next = fu;
        }
        break; //return head;
      }

      p1 = p0;
      p0 = p0->next; 
    }

    // Append the node at the end of the list
    p1->next = fu;
    fu->next = NULL;
 }   

 FuGoTable[opType][fuNu] = head;
}
*/

