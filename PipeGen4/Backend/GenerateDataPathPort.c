#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "Schedule.h"
#include "PortSchedule.h"

#ifdef DATA_PATH_ONLY

#define PORT_NAME_LIST 0
#define PORT_DECL_LIST 1

void PortListInsert(int name_or_decl, char *s);
void FreePortList();

int GenerateMuxSelPort(int SharedRegNu) {

  enum operation op_type;
  int MuxCnt = 0;

  int fu_nu, port_nu;
  MuxPtr Mux;
  int size;

  int regNu;
  int muxSelWidth;
  int muxInputNu;
  ResourceList fu, head;
  char name[30];

// phase1 mux
  for (op_type = add; op_type < add + OP_NU; op_type++) {
    size = (*RQ[op_type])->size;
    for (fu_nu = 0; fu_nu < size; fu_nu++) {
      for (port_nu = 0; port_nu < 2; port_nu++) {

         Mux = (MuxRAT[op_type][fu_nu]).Fu_portNu[port_nu];
         while (Mux != NULL) {
           //fprintf(vpp, "s%d,\n", Mux->m);
           //fprintf(vpp, "input s%d;\n", Mux->m);
           name[0] = '\0';
           sprintf(name, "s%d,", Mux->m);
           myprintf("name = %s\n", name);
           PortListInsert(PORT_NAME_LIST, name);

           name[0] = '\0';
           sprintf(name, "input          s%d;", Mux->m);
           myprintf("name = %s\n", name);
           PortListInsert(PORT_DECL_LIST, name);

           MuxCnt++;
           Mux = Mux->next;
         }
      }
    }
  }

// phase2 mux
  for (regNu = 1; regNu <= SharedRegNu; regNu++) {
    muxInputNu = 0;
    fu = head = FuRAT[regNu];

    while (fu != NULL) {
      fu = fu->next;
      muxInputNu++;
    }

    if (muxInputNu >= 2) {

      MuxCnt++;

      muxSelWidth = IntLog2(muxInputNu);
      
      //fprintf(vpp, "s%d,\n",  MuxCnt);
      //fprintf(vpp, "input [%d : 0] s%d;\n",  muxSelWidth - 1, MuxCnt);

      name[0] = '\0';
      sprintf(name, "s%d,",  MuxCnt);
      myprintf("name = %s\n", name);
      PortListInsert(PORT_NAME_LIST, name);

      name[0] = '\0';
      sprintf(name, "input [%d : 0] s%d;",  muxSelWidth - 1, MuxCnt);
      myprintf("name = %s\n", name);
      PortListInsert(PORT_DECL_LIST, name);
    }
  }
}

void GenerateRegFileEnablePort(int regNu, 
                               RF_portPtr RegFileTable,
                               int DestNu) {
  char idx;
  int i, j;
  int RF_PortNu;
  RF_NuCnt head;
  char name[30];

  //fprintf(vpp, "r%d_wen,\n" , regNu);
  //fprintf(vpp, "input          r%d_wen;\n" , regNu);

  name[0] = '\0';
  sprintf(name, "r%d_wen," , regNu);
  myprintf("name = %s\n", name);
  PortListInsert(PORT_NAME_LIST, name);

  name[0] = '\0';
  sprintf(name, "input          r%d_wen;" , regNu);
  myprintf("name = %s\n", name);
  PortListInsert(PORT_DECL_LIST, name);

  RF_PortNu = TotalRegFilePortNu(RegFileTable, DestNu);

  for (i = 1; i <= RF_PortNu; i++) {
    head = RegFileTable[i - 1];
    assert (head.RegFileList != NULL);

    if (head.sharedCnt == 1) {
      // port declarations
      //fprintf(vpp, "R%d_enable_%d,\n", regNu, i);
      //fprintf(vpp, "input R%d_enable_%d;\n", regNu, i);

      name[0] = '\0';
      sprintf(name, "R%d_enable_%d,", regNu, i);
      myprintf("name = %s\n", name);
      PortListInsert(PORT_NAME_LIST, name);

      name[0] = '\0';
      sprintf(name, "input          R%d_enable_%d;", regNu, i);
      myprintf("name = %s\n", name);
      PortListInsert(PORT_DECL_LIST, name);

    } else {

      assert (head.sharedCnt > 1);

      // Instantiate multiple address counters to read RF 
      for (j = 0; j < head.sharedCnt; j++) {
        idx = 'a' + j;

        // port declarations
        //fprintf(vpp, "R%d_enable_%d%c,\n", regNu, i, idx);
        //fprintf(vpp, "input R%d_enable_%d%c;\n", regNu, i, idx);
        name[0] = '\0';
        sprintf(name, "R%d_enable_%d%c,", regNu, i, idx);
        myprintf("name = %s\n", name);
        PortListInsert(PORT_NAME_LIST, name);

        name[0] = '\0';
        sprintf(name, "input          R%d_enable_%d%c;", regNu, i, idx);
        myprintf("name = %s\n", name);
        PortListInsert(PORT_DECL_LIST, name);
      }
    }
  }
}

void GenerateRegEnablePort(int regNu) {
  // port declarations
  char name[30];

  name[0] = '\0'; // *name = '\0';
  sprintf(name, "r%d_wen,", regNu);
  myprintf("name = %s\n", name);
  PortListInsert(PORT_NAME_LIST, name);

  name[0] = '\0';
  sprintf(name, "input          r%d_wen;" , regNu);
  myprintf("name = %s\n", name);
  PortListInsert(PORT_DECL_LIST, name);

  //fprintf(vpp, "r%d_wen,\n", regNu);
  //fprintf(vpp, "input r%d_wen;\n", regNu);
}


void FreePortList() {

  portStringPtr p1, p2, p, head;
  p = head = PortNameList;

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
      //printf("free string %s\n", p1->s);
      free(p1->s);
      free(p1);
      p = head;
    } else {
      free(p->s);
      free(p);
      break; 
    }
  }
  PortNameList = NULL;

  p = head = PortDeclList;

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
      //printf("free string %s\n", p1->s);
      free(p1->s);
      free(p1);
      p = head;
    } else {
      free(p->s);
      free(p);
      break; 
    }
  }

  PortDeclList = NULL;
}

// if name_or_decl = 0, use PORT_NAME_LIST 
// if name_or_decl = 1, use PORT_DECL_LIST 
void PortListInsert(int name_or_decl, char *s) {

  portStringPtr p0, p1, head;
  portStringPtr port;

  p0 = head = name_or_decl ? PortDeclList : PortNameList; // global

  port = malloc (sizeof(struct portString));
  port->s = strdup(s);

  if (port->s == NULL) perror("error in strdup");

  if (head == NULL) {
    head = port;
    port->next = NULL;
  } else {

    while (p0 != NULL) {
      p1 = p0;
      p0 = p0->next;
    }

    // Append at the end
    p1->next = port;
    port->next = NULL;
  }   

  if (name_or_decl) 
    PortDeclList = head;
  else 
    PortNameList = head;
}

#endif //#ifdef DATA_PATH_ONLY
