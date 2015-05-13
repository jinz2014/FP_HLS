#include <stdio.h>
#include <stdlib.h>
#include "Schedule.h"

//===================================================================================
// Since all top-level data outputs are connected to the outputs of functional units,
// we can just have the outputs of FUs registered and connected the registered
// outputs to the top-level outputs. 
//
// Note each ready output is controlled by a counter.  The counter is incremented by 
// the ready signal. Since some ready signals may be asserted at the same time, we 
// can use a counter for several ready outputs rather than one ready output.
//===================================================================================

void GenerateOutputAssign (FILE *vfp, char *ModuleName,
   const char *SignalName1, const char *SignalName2, const char *SignalName3) {

   int i;
   int outputRdyTime; 
   int resourceNu;
   int outputCnt;
   OutputRdyList head = NULL;
   OutputRdyList head_cpy;
   char *opSym;
   enum operation op_type;
   int size;
   int dataWidth;

  // We should count the FU output register that drives the design output!
   int **sinkResourceTable = (int **) malloc (sizeof(int*) * OP_NU);
   for (op_type = add; op_type < add + OP_NU; op_type++) {
     size = (*RQ[op_type])->size;
     sinkResourceTable[op_type] = (int *) calloc (size, sizeof(int));
   }

   for (i = 0; i < NODE_NU; i++)
     if (DFG[i]->opDest[0] == SINK) {
       resourceNu = DFG[i]->opResourceNu;
       if (sinkResourceTable[DFG[i]->op][resourceNu-1] == 0) {
         sinkResourceTable[DFG[i]->op][resourceNu-1] = 1;
         dataWidth = DataPathWidth(DFG[i]->op, 0);
         CSP->reg += dataWidth;
       }
     }

   /*
   myprintf("Sink resource Table (FUs that drive the design outputs)\n");
   for (op_type = add; op_type < add + OP_NU; op_type++) {
     size = (*RQ[op_type])->size;
     myprintf("%s " opSymTable(op_type));
     for (i = 0; i < size; i++)
       myprintf("%d " sinkResourceTable[op_type][i]);
     myprintf("\n");
   }
   */

  //===================== signal declarations =======================
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    opSym = opSymTable(op_type);
    size = (*RQ[op_type])->size;
    dataWidth = DataPathWidth(op_type, 0);
    for (i = 0; i < size; i++) {
      fprintf(vfp, "reg [%d : 0] %s%d_out_r;\n", dataWidth - 1, opSym, i+1);
    }
  }

  for (i = 0; i < NODE_NU; i++)
    if (DFG[i]->opDest[0] == SINK) {
      outputRdyTime = DFG[i]->opResultDoneSlot;
      head = OutputRdyListInsert(head, outputRdyTime, &outputCnt);
    }

  head_cpy = head; 

  for (i = 0; i < outputCnt; i++) {
    //fprintf(vfp, "reg [31:0] %s_out%d_cnt;\n", ModuleName, i);
    fprintf(vfp, "reg  %s_out%d_rdy_r;\n", ModuleName, i);
    // 
    CSP->reg += 1;
  }
  for (i = 0; i < outputCnt; i++) {
    fprintf(vfp, "wire %s_out%d_rdy;\n", ModuleName, i);
  }

  fprintf(vfp, "\n");

  /*===================== output ready count =======================
  fprintf(vfp, "always @ (posedge clk) begin\n");
  fprintf(vfp, "  if (rst) begin\n");
  for (i = 0; i < outputCnt; i++) {
    fprintf(vfp, "  %s_out%d_cnt <= 0;\n", ModuleName, i);
  }
  fprintf(vfp, "  end\n", outputCnt);
  fprintf(vfp, "  else begin\n");

  for (i = 0; i < outputCnt; i++) {
    fprintf(vfp, "  if (!stall && %s_out%d_rdy) begin\n", ModuleName, i);
    fprintf(vfp, "    %s_out%d_cnt <= %s_out%d_cnt + 1'b1;\n", 
        ModuleName, i, ModuleName, i);
    fprintf(vfp, "  end\n");
  }
  fprintf(vfp, "  end\n");
  fprintf(vfp, "end\n\n");
  */

  //===================== assign output ready  =======================
  while (head != NULL) {

    //fprintf(vfp, "assign %s_out%d_rdy = state[%d] & (%s_out%d_cnt < TEST_NU);\n",\
        ModuleName, head->outputRdyNu, head->outputRdyTime, ModuleName, head->outputRdyNu);

    fprintf(vfp, "assign %s_out%d_rdy = state[%d];\n",
        ModuleName, head->outputRdyNu-1, head->outputRdyTime);

    head = head->next;
  }

  //==================== registered output ready ===================
  head = head_cpy;
  fprintf(vfp, "always @ (posedge clk) begin\n");
  while (head != NULL) {

    fprintf(vfp, "  %s_out%d_rdy_r <= ~rst & %s_out%d_rdy;\n",
        ModuleName, head->outputRdyNu-1, 
        ModuleName, head->outputRdyNu-1);

    head = head->next;
  }
  fprintf(vfp, "end\n\n");



  //================= registered FU output result ====================
  fprintf(vfp, "always @ (posedge clk) begin\n");
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    opSym = opSymTable(op_type);
    size = (*RQ[op_type])->size;
    for (i = 0; i < size; i++) {
      fprintf(vfp, "  %s%d_out_r <= %s%d_out;\n", opSym, i+1, opSym, i+1);
    }
  }
  fprintf(vfp, "end\n\n");


  // ============ Final design output assignments ============
  for (i = 0; i < NODE_NU; i++)
    if (DFG[i]->opDest[0] == SINK) {
      outputRdyTime = DFG[i]->opResultDoneSlot;
      resourceNu = DFG[i]->opResourceNu;
      opSym = opSymTable(DFG[i]->op);
      head = head_cpy; // for each sink find its output ready time
      while (head != NULL) {
        if (outputRdyTime == head->outputRdyTime) {
          fprintf(vfp, "assign %s_%s_%s_%s%s = %s_out%d_rdy_r;\n",
              ModuleName, DFG[i]->opName, 
              SignalName1, SignalName2, SignalName3,
              ModuleName, head->outputRdyNu-1);

          fprintf(vfp, "assign %s_%s_%s%s = %s%d_out_r;\n", 
              ModuleName, DFG[i]->opName, 
              SignalName1, SignalName3,
              opSym, resourceNu);
        }
        head = head->next;
      }
    }

  fprintf(vfp, "\n\n");

  for (op_type = add; op_type < add + OP_NU; op_type++)
    free(sinkResourceTable[op_type]);
  free(sinkResourceTable);

  while (head_cpy != NULL) { 
    head = head_cpy;
    head_cpy = head->next;
    free(head);
  }
}


// Use the list to sort the unique number of output ready time 
OutputRdyList OutputRdyListInsert(OutputRdyList head, int outputRdyTime, int *outputCnt) {

  OutputRdyList t, p0, p1, p2;

  p0 = p2 = head;

  /* Check resource op and number are unique */  
  while (p2 != NULL) {
    if (p2->outputRdyTime == outputRdyTime)
      return head;
    else
      p2 = p2->next;
  }

  t = malloc(sizeof(struct OutputRdy));
  t->outputRdyTime = outputRdyTime;

  if (head == NULL) {
    head = t;
    t->next = NULL;
    t->outputRdyNu = 1;
  } else {
    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }

    p1->next = t;
    t->next = NULL;
    t->outputRdyNu = p1->outputRdyNu + 1;
  }
  // the total number of counts
  *outputCnt = t->outputRdyNu;

  return (head);
}

void OutputRdyListPrint(OutputRdyList head) {
  OutputRdyList p = head;
  while (p != NULL) {
    myprintf("output ready num = %d output ready time = %3d\n", 
    p->outputRdyTime, p->outputRdyNu);
    p = p->next;
  }
}

