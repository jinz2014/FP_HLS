#ifndef PORT_SCHEDULE_
#define PORT_SCHEDULE_

// Ports node (16 inputs at max)
#define P1   -1
#define P2   -2
#define P3   -3
#define P4   -4
#define P5   -5
#define P6   -6
#define P7   -7
#define P8   -8
#define P9   -9 
#define P10  -10
#define P11  -11
#define P12  -12
#define P13  -13
#define P14  -14
#define P15  -15
#define P16  -16

// Sink node
#define SINK -1

#include "Schedule.h"

#define PORT_CONSTRAINT

typedef struct {
  int dly; // circuit delay
  int reg; // circuit total regs
  int mux_reg; // circuit total mux chain register
  int mux; // circuit total muxs
  int muxp; // circuit total muxps
  int muxr; // circuit total muxrs
  int fan; // circuit max mux fan-in
  int rfo; // circuit max reg fan-out
  int ffo; // circuit max fu fan-out
  int pfo; // circuit max port fan-out
  int tfo; // circuit total fan-out
  int fanout; // circuit max mux fan-out
} CircuitStats, *CircuitStatsPtr;

typedef struct {
  int min_dly; int max_dly;
  int min_mux; int max_mux;
  int min_fan; int max_fan;
  int min_reg; int max_reg;
  int min_rfo; int max_rfo;
  int min_pfo; int max_pfo;
  int min_ffo; int max_ffo;
  int min_tfo; int max_tfo;
  int min_fanout; int max_fanout;
} ScheduleStats, *ScheduleStatsPtr;


#ifdef PORT_CONSTRAINT

//extern int PORTS_CONSTRAINT[MAX_PORT_NU];
extern int PORTS_CONSTRAINT[];

extern int Xcomb;
extern int KComb;

#endif // #ifdef PORT_CONSTRAINT

/* operator subtree coverage */
void SetPortCover(int coverage[]);

/* FU inputs shared by the input port nodes */
void SetPortFU(int frequency[]);

/* Number of successors from primary input to output */
void SetPortSucc(int successor[]);

void PortCover(int nodeID, int *coverage);
int PathSucc(int nodeID);
int PathLength(int nodeID);

//void CreatePortPriority(int maxInputPortNu);
void CreatePortPriority(int maxInputPortNu, int P);

int CheckPortCover(int op, int i);
int OpCover(int op, int i);

void SetPortPriority(int a0[], int a1[], int a2[], int p[], int maxInputPortNu);

//void PortDirectedSchedule(FILE *fp, int portConfigCnt, int maxPortCombo, int *ASAP_slots, int *ALAP_slots, \
    int *Xconstraint, int *Kconstraint, int px, int pk, ScheduleStatsPtr SSP);

void PortDirectedSchedule(FILE *fp, int portConfigCnt, int maxPortCombo, 
                          int *ASAP_slots, int *ALAP_slots, 
                          int *Xconstraint, int *Kconstraint, 
                          ScheduleStats SS[]);

int GetPortPriority(int nodeID, int PortPriority[], int maxInputPortNu);

//int GetInputPortNu(int regNu, int portNu);
int GetInputPortNu(int portNu);

int fac(int n);
int perm(int a[], int a_size);

//===========================================================================
// HDL 
//===========================================================================

/* */
void GenerateInputPorts(FILE *vfp, const char *ConnectionName, int dataWidth);

/* */
void GenerateOutputPorts(FILE *vfp, char *ModuleName, const char *ConnectionName, 
                         const char *SignalName1, const char *SignalName2, 
                         const char *SignalName3);

#endif /* PORT_SCHEDULE_ */
