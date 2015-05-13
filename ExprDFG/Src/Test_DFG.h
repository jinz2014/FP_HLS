#ifndef TEST
#define TEST_

enum operation {add, mul, div_, nop};

typedef struct RF_Nu *RF_portNuPtr;

struct RF_Nu {
  int sharedDestNu;
  int *sharedDestNuPtr;
  RF_portNuPtr next;
};

typedef struct {
  RF_portNuPtr RegFileList;
  int sharedCnt;
} RF_NuCnt, *RF_portPtr;

/* Operation Node */
typedef struct {
  //int opID;
  enum operation op;
  int opLatency; 
  int opRate; 
  int *opSrc;
  int *opDest;
  int opDestNu;
  //RF_portNuPtr *RegFileList;
  RF_portPtr RegFileTable;
  char opConst;
  float opConstVal;
  int opScheduledSlot;
  int opPriority;
  int opResourceNu;
  char opResultDone;
  int opResultDoneSlot;
  char opScheduled;
  float *opVal;
} opNode;

typedef union {
  int i;
  float f;
} int32_or_float;

extern const int SRC;
extern const int SINK;
extern const int P1, P2, P3, P4, P5, P6, P7, P8;
extern const int P9, P10, P11, P12, P13, P14, P15, P16;

extern char *CircuitName;
extern int  maxPortNu;
extern int  NODE_NU;
extern opNode **DFG;

#endif /* SCHEDULE_ */
