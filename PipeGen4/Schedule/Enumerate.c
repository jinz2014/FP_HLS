#include "Schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
//
// Notes:
// xi: operation node xi whose id is i 
//
// i starts off at MAX_PORT_NU
//
void Enumerate(int *ASAP_slots, int *ALAP_slots, int i) {

  int first, last;
  int step;
  int cur_latency;
  int cur_mux_fanin;
  int cur_mux_fanout = 0;
  int cur_mux_input;
  int cur_muxp_input;
  int cur_muxr_input;
  int cur_reg_nu;
  int fuNu; // functional unit number
  static int best_latency = 100000;

  //------------------------------------------------------------
  // vars to support GenerateBackEnd
  char scheduleName[10];
  ScheduleStats SS[3]; // ASAP, ALAP, Iter
  //------------------------------------------------------------

  if (i == NODE_NU) {
    myprintf("enumeration hit the last node\n");

    CheckConflict(); 

    if (min_mux_fanin == 2) {
      fprintf(stderr, "\nfanin = 2\n");
      fprintf(stderr, "Cur Enum mux fanout (%d %d)\n", min_mux_fanout, max_mux_fanout);
      fprintf(stderr, "Cur Enum mux fanin (%d %d)\n", min_mux_fanin, max_mux_fanin);
      fprintf(stderr, "Cur Enum muxp input (%d %d)\n", min_muxp_input, max_muxp_input);
      fprintf(stderr, "Cur Enum muxr input (%d %d)\n", min_muxr_input, max_muxr_input);
      fprintf(stderr, "Cur Enum mux input (%d %d)\n", min_mux_input, max_mux_input);
      fprintf(stderr, "Cur Enum reg number(%d %d)\n", min_reg_nu, max_reg_nu);
      fprintf(stderr, "Cur Enum latency   (%d %d)\n", min_latency, max_latency);
      PrintDFG(ASAP_slots, ALAP_slots); exit(0);
    }

    if (max_mux_fanin == 6) {
      fprintf(stderr, "\nfanin = 6\n");
      fprintf(stderr, "Cur Enum mux fanout (%d %d)\n", min_mux_fanout, max_mux_fanout);
      fprintf(stderr, "Cur Enum mux fanin (%d %d)\n", min_mux_fanin, max_mux_fanin);
      fprintf(stderr, "Cur Enum muxp input (%d %d)\n", min_muxp_input, max_muxp_input);
      fprintf(stderr, "Cur Enum muxr input (%d %d)\n", min_muxr_input, max_muxr_input);
      fprintf(stderr, "Cur Enum mux input (%d %d)\n", min_mux_input, max_mux_input);
      fprintf(stderr, "Cur Enum reg number(%d %d)\n", min_reg_nu, max_reg_nu);
      fprintf(stderr, "Cur Enum latency   (%d %d)\n", min_latency, max_latency);
      //PrintDFG(ASAP_slots, ALAP_slots); exit(0);
    }

    ENUM_NU++;

    if (fmod(ENUM_NU, 1000) == 0) {
      //fprintf(stderr, "progress %lf%\n", ENUM_NU / ENUM_SZ * 100);
      fprintf(stderr, "progress %lf\n", ENUM_NU);
      fprintf(stderr, "Cur Enum mux fanout (%d %d)\n", min_mux_fanout, max_mux_fanout);
      fprintf(stderr, "Cur Enum mux fanin (%d %d)\n", min_mux_fanin, max_mux_fanin);
      fprintf(stderr, "Cur Enum muxp input (%d %d)\n", min_muxp_input, max_muxp_input);
      fprintf(stderr, "Cur Enum muxr input (%d %d)\n", min_muxr_input, max_muxr_input);
      fprintf(stderr, "Cur Enum mux input (%d %d)\n", min_mux_input, max_mux_input);
      fprintf(stderr, "Cur Enum reg number(%d %d)\n", min_reg_nu, max_reg_nu);
      fprintf(stderr, "Cur Enum latency   (%d %d)\n", min_latency, max_latency);

      printf("stopped at 10000\n"); exit(0);

      if (ENUM_NU > 1000000) return;
      //if (ENUM_NU == 60000) getchar();
    }

    // Debug spot

    // 
    cur_latency = DFG[NODE_NU-1]->opScheduledSlot + DFG[NODE_NU-1]->opLatency + 1;
    myprintf("current latency = %d\n", cur_latency);

    if (min_latency > cur_latency) {
      min_latency = cur_latency;
    }

    if (max_latency < cur_latency) {
      max_latency = cur_latency;
    }

    // 
    GenerateBackEnd(stderr, scheduleName, "Iter", PORT_NU, cur_latency, SS);

    if (cur_mux_fanout < CSP->rfo) cur_mux_fanout = CSP->rfo;
    if (cur_mux_fanout < CSP->pfo) cur_mux_fanout = CSP->pfo;
    if (cur_mux_fanout < CSP->ffo) cur_mux_fanout = CSP->ffo;

    cur_mux_fanin = CSP->fan;
    cur_mux_input = CSP->mux;
    cur_muxp_input = CSP->muxp;
    cur_muxr_input = CSP->muxr;
    cur_reg_nu    = CSP->reg;

    // best_mux_fanout
    if (min_mux_fanout > cur_mux_fanout) {
      min_mux_fanout = cur_mux_fanout;
    }

    if (max_mux_fanout < cur_mux_fanout) {
      max_mux_fanout = cur_mux_fanout;
    }

    // best_reg_nu
    if (min_reg_nu > cur_reg_nu) {
      min_reg_nu = cur_reg_nu;
    }

    if (max_reg_nu < cur_reg_nu) {
      max_reg_nu = cur_reg_nu;
    }

    // best_mux_input
    if (min_mux_input > cur_mux_input) {
      min_mux_input = cur_mux_input;
    }

    if (max_mux_input < cur_mux_input) {
      max_mux_input = cur_mux_input;
    }

    // best_muxp_input
    if (min_muxp_input > cur_muxp_input) {
      min_muxp_input = cur_muxp_input;
    }

    if (max_muxp_input < cur_muxp_input) {
      max_muxp_input = cur_muxp_input;
    }

    // best_muxr_input
    if (min_muxr_input > cur_muxr_input) {
      min_muxr_input = cur_muxr_input;
    }

    if (max_muxr_input < cur_muxr_input) {
      max_muxr_input = cur_muxr_input;
    }

    // best_mux_fanin
    if (min_mux_fanin > cur_mux_fanin) {
      min_mux_fanin = cur_mux_fanin;
    }

    if (max_mux_fanin < cur_mux_fanin) {
      max_mux_fanin = cur_mux_fanin;
    }
  }

  else {

      first = ASAP_slots[i];
      last = ALAP_slots[i];
    /*
    if (ASAP_slots[i] < ALAP_slots[i]) {
      first = ASAP_slots[i];
      last = ALAP_slots[i];
    } 
    else {
      // ALAP has shorter latency 
      first = ALAP_slots[i];
      last = ASAP_slots[i];

      // update ASAP/ALAP slots
      ASAP_slots[i] = first;
      ALAP_slots[i] = last;
    }
    */

    int step_size, step_range;

    assert(last >= first);

    step_range = last - first + 1;

    /* ordbbr
      if (step_range > 60)
        step_size = 30; //10;
      else if (step_range > 40)
        step_size = 20; //10;
      else  if (step_range > 30)
        step_size = 15; //8;
      else  if (step_range > 20)
        step_size = 10; //6;
      else  if (step_range > 10)
        step_size = 5;
      else  if (step_range > 2)
        step_size = 2;
      else 
        step_size = 1;

    // for ordbur, ppbr
      if (step_range > 40)
        step_size = 10;
      else  if (step_range > 30)
        step_size = 8;
      else  if (step_range > 20)
        step_size = 6;
      else  if (step_range > 10)
        step_size = 4;
      else  if (step_range > 2)
        step_size = 2;
      else 
        step_size = 1;
        */

        step_size = 1;

    myprintf("enum %lf debug mobility step: %d [%d %d]\n", ENUM_NU, step_size, first, last);

    //for (step = first; step <= last; step++) {
    for (step = first; step <= last; step += step_size) {

      myprintf("Try step %d of DFG node %d [%d %d]\n", step, i, first, last);

      // save ASAP values
      int *save_ASAP_slots = (int *) malloc (sizeof(int) * NODE_NU);
      int *save_ALAP_slots = (int *) malloc (sizeof(int) * NODE_NU);
      memcpy(save_ASAP_slots, ASAP_slots, NODE_NU * sizeof(int));
      memcpy(save_ALAP_slots, ALAP_slots, NODE_NU * sizeof(int));

      // ResourceUsed (step, type(xi))
      // Increment ResourceUsed (step, type(xi))
      if ((fuNu = InsertScheduleRAT(DFG[i]->op, step)) != -1) {

        // S(i) = step 
        // Specificaly, update the operator's start and end time
        DFG[i]->opScheduledSlot = step;
        DFG[i]->opResultDoneSlot = step + DFG[i]->opLatency;
        DFG[i]->opResourceNu = fuNu + 1;
        myprintf("DFG node %d has resourceNu %d\n", i, fuNu);

        UpdateASAP(i, ASAP_slots, ALAP_slots, step);

        myprintf("Start enumerating at DFG node %d\n", i+1);
        Enumerate(ASAP_slots, ALAP_slots, i+1);
        myprintf("Return from enumerating at DFG node %d\n", i+1);

        DecScheduleRAT(i, step);
      }

      // restore ASAP values
      memcpy(ASAP_slots, save_ASAP_slots, NODE_NU * sizeof(int));
      memcpy(ALAP_slots, save_ALAP_slots, NODE_NU * sizeof(int));
      free(save_ASAP_slots);
      free(save_ALAP_slots);
    }
  }
}

void DecScheduleRAT(int nu, int slot) {
  int i;
  int fuNu;
  ScheduledSlotPtr head, p0, p1;

  int op_type = DFG[nu]->op;
  int resource_nu = DFG[nu]->opResourceNu;

  fuNu = (GetOpSym(op_type) == 'm' || GetOpSym(op_type) == 'r') ?
         FU[op_type] : myceil(FU[op_type], minII);

  // find the FU and slot to which the operation is assigned 
  for (i = 0; i < fuNu; i++) {
    p1 = p0 = SRAT[op_type][i].head;
    while (p0 != NULL) {
      if (p0->slot == slot && resource_nu == i + 1) {
        (SRAT[op_type][i]).slotNu--;
        myprintf("Decrement %s%d at slot %d\n", opSymTable(op_type), i, slot);
        // delete the list elem
        p1->next = p0->next;
        if ((SRAT[op_type][i]).slotNu == 0) (SRAT[op_type][i]).head = NULL;
        free(p0);
        return;
      }
      p1 = p0;
      p0 = p0 -> next;
    }
  }
}

void UpdateASAP(int i, int *ASAP_slots, int *ALAP_slots, int step) {
  int actual, orig;
  int destNu;
  int j, k;

  while(1) {

    if (DFG[i]->opDest[0] == SINK) return;

    // register insertion
    //actual = ASAP_slots[i] + DFG[i]->opLatency + 1 + 1; 
    actual = step + DFG[i]->opLatency + 1 + 1; // register insertion?

    destNu = DFG[i] -> opDestNu;

    for (j = 0; j < destNu; j++) {
      k = DFG[i]->opDest[j];
      orig = ASAP_slots[k];
      myprintf("Update ASAP: node %d's dest %d orig = %d actual = %d\n", i, k, orig, actual);
      if (actual > orig && actual <= ALAP_slots[k]) {
        myprintf("Update ASAP: node %d orig = %d actual = %d\n", \
            k, orig, actual);
        ASAP_slots[k] = actual;
        UpdateASAP(k, ASAP_slots, ALAP_slots, actual);
      }
    }
    return;
  }
}
