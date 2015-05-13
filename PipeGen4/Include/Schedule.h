#ifndef SCHEDULE_
#define SCHEDULE_

#include <stdio.h>
#include "PortSchedule.h"
#include "queue.h"

/****************************************************/
/* Generate MUX chain registers in front of FU op   */
/****************************************************/
//#define ADD_MUX_REG_DIV
//#define ADD_MUX_REG_ADD
//#define ADD_MUX_REG_MUL
//#define ADD_MUX_REG_SUB
////
/****************************************************/
/* using -DRC_SCHEDULE -DMC_SCHEDULE in command line*/
/*                                                  */
/* minimum-resource schduling                       */
//#define RC_SCHEDULE

// minimum-delay schduling
//#define MC_SCHEDULE
/****************************************************/

/****************************************************/
/* automatically replaced by sed                    */
#define BENCHMARK
/****************************************************/

/****************************************************/
/* dump lots of info                                */
#define DEBUG
/****************************************************/

/****************************************************/
/* dump pipeline results in hex as well as float    */
//#define HEX_DUMP
/****************************************************/

/****************************************************/
/* execute modelsim simulation after HDL generation */
//#define MODELSIM
/****************************************************/

/****************************************************/
/* Generate Modelsim testbench and do file          */
//#define TB

/* Generate stall cycles to the DUT in the testbench*/
//#define STALL
/****************************************************/

/****************************************************/
/* load or generate hardware test stimulus          */
//#define LOAD_TEST_DATA
/****************************************************/

/****************************************************/
/* register allocation approach                     */
/*                                                  */
/* using -DWBM, -DLEA, -DALL in the command line    */
/****************************************************/

/****************************************************/
/* Generate datapath of the circuit only            */
/*                                                  */
//#define DATA_PATH_ONLY
/****************************************************/


/****************************************************/
/* Floating-point/integer operation                 */
#define FLOATING_POINT
/****************************************************/

/****************************************************/
/* By default: topological input ports scheduling   */
/*                                                  */
/* Ishimori's priority                              */ 
/* #define PORT_PRIORITY                            */
/*                                                  */
/* Exhaustive port priority enumeration             */
/* #define PORT_ALL_PRIORITY                        */
/* See RC_Scheduled.c                               */
/*                                                  */
/*#define PORT_PRIORITY                             */
/*#define PORT_ALL_PRIORITY                         */
/* using -DPORT_PRIORITY -DPORT_ALL_PRIORITY        */
/****************************************************/

/****************************************************/
/* Controller                                       */
/* Generate control datapath                        */
/*                                                  */
#define CONTROL_PATH
/****************************************************/

/****************************************************/
/* Dump circuits statistics into csv file           */
//#define CSV
/****************************************************/

/****************************************************/
/* Total number of test counts                      */
#define TEST_CNT     1
/****************************************************/

/****************************************************/
/* Application-specific operations                  */

/*
#define OP_NU 6
enum operation {add, 
                sub, 
                mul,
                div_,
                max, 
                gt,
                nop,
                // tmax unused (6 outputs)
                addd, 
                muld, 
                tod,
                tof,
                gtd, 
                mx,
                mxd,
                romd, 
                nopc,
                subd,
                rom,
                maxd,
                divd,
                lt,
                ltd};
                */

#define OP_NU 3
enum operation {add, 
                sub, 
                mul,
                nop,
                // power unused
                nopc,
                addd, 
                muld, 
                div_,
                tod,
                tof,
                gt,
                gtd, 
                mx,
                mxd,
                subd,
                rom,
                romd, 
                max, 
                maxd,
                divd,
                lt,
                ltd};

/*
#define OP_NU 3
enum operation {add, 
                mul, 
                div_,
                nop,
                // uuci unused
                addd, 
                muld, 
                tod,
                tof,
                gt,
                gtd, 
                max, 
                mx,
                mxd,
                romd, 
                nopc,
                sub, subd, rom, maxd, divd, lt, ltd};
                */

/*
#define OP_NU 2
enum operation {add, 
                mul,
                nop,
                // power unused
                nopc,
                addd, 
                muld, 
                div_,
                tod,
                tof,
                gt,
                gtd, 
                mx,
                mxd,
                sub, 
                subd,
                rom,
                romd, 
                max, 
                maxd,
                divd,
                lt,
                ltd};
                */

/*
#define OP_NU 3
enum operation {add, 
                mul, 
                mx,
                nop,
                // mux2 unused
                addd, 
                muld, 
                div_,
                tod,
                tof,
                gt,
                gtd, 
                max, 
                mxd,
                romd, 
                nopc,
                sub, subd, rom, maxd, divd, lt, ltd};
                */

/*
#define OP_NU 4
enum operation {add, 
                sub,
                mul, 
                div_,
                nop,
                // ppbr,ordbbr unused
                addd, 
                muld, 
                tod,
                tof,
                gt,
                gtd, 
                max, 
                mx,
                mxd,
                romd, 
                nopc,
                subd, rom, maxd, divd, lt, ltd};
*/

/*
#define OP_NU 4
enum operation {add, tod, addd, muld, nop,
  // f2d unused 
                sub, mul, div_, lt, gt, rom,
                subd, divd, ltd, gtd, romd, 
                mx, mxd};
*/

/*
#define OP_NU 7
enum operation {add, 
                mul,
                div_,
                gt,
                max, 
                mx,
                rom,
                nop,
                nopc,
                // mrbay unused 
                addd, muld, tod, tof, gtd, mxd,  
                sub, subd, romd, maxd, divd, lt, ltd};
#define OP_NU 9
enum operation {add, 
                addx,  // custom precision (24+11)
                mul,
                mulx,  // custom precision (24+11)
                div_,
                gt,
                max, 
                mx,
                rom,
                nop,
                nopc,
                // mrbay unused 
                addd, muld, tod, tof, gtd, mxd,  
                sub, subd, romd, maxd, divd, lt, ltd};
*/

/****************************************************/

/****************************************************/
/* floating-pont precision                          */
enum precision {sfp, dfp, bw1, und};
/****************************************************/

/****************************************************/
/* A simple ROM orgnization                         */
#define DEPTH 16
#define WORDS 5
/****************************************************/

enum priority {LENGTH, MOBILITY, NONE};


/* Register File read port number */
typedef struct RF_Nu *RF_portNuPtr;

struct RF_Nu {
  int sharedDestNu;
  int *sharedDestNuPtr;
  RF_portNuPtr next;
} RF_DestNu;

typedef struct {
  int depth;
  int *entry;
} FanoutR, *FanoutR_ptr;

typedef struct {
  RF_portNuPtr RegFileList;
  int sharedCnt;
} RF_NuCnt, *RF_portPtr;

/* Operation Node */
typedef struct {
  enum precision opPrecision;
  enum operation op;
  int opLatency; 
  int opRate; 
  int *opSrc;
  int opSrcNu;
  int *opDest;
  int opDestNu;
  //RF_portNuPtr *RegFileList;
  RF_portPtr RegFileTable;
  char opConst;
  int opScheduledSlot;
  int opPriority;
  int opResourceNu;
  char opResultDone;
  int opResultDoneSlot;
  char opScheduled;
  float opConstVal;
  double opConstVal_d;
  float *opMuxVal;
  double *opMuxVal_d;
  #ifdef FLOATING_POINT
  float *opVal;
  double *opVal_d;
  #else
  int *opVal;
  #endif
  char *opName; // 8/30/12
} opNode;

//==============================================
// Schedule RAT
//==============================================
typedef struct ScheduledSlot *ScheduledSlotPtr;

struct ScheduledSlot{
 int slot;
 ScheduledSlotPtr next;
};

typedef struct {
  int conflict;
  int slotNu;
  ScheduledSlotPtr head;
} SRAT_FuType, *SRAT_FuType_Ptr;
//==============================================
// Schedule RAT end
//==============================================


/* */
typedef struct OpAvailable *List;

struct OpAvailable {
  int nodeID;
  int priority;
  List next;
};

/* */
struct conflict_table {
  int count;
  int conflicts[1000];
  int II;
};

/* Variable life time */
typedef struct varLifeTime *varList; 
struct varLifeTime {
  int varID;
  int startTime;
  int endTime;
  varList next;
};

typedef struct {
  int varID;
  int fuNu;
  enum operation fuOp;
} var, *var_ptr;

typedef struct {
  var_ptr var_list; 
  int var_nu;
} var_struct, *var_struct_ptr; 

typedef struct portString *portStringPtr; 
struct portString {
  char *s;
  portStringPtr next;
};

/* Some members may be reserved for future use */
typedef struct {
  int *destNu;
  float *constNu;
  double *constNu_d;
  int *regNu;
  int *muxNu;
  int *fuNu;
} MuxInputs, *MiPtr;

/* Some members may be reserved for future use */
typedef struct {
  int *regNu;
  int *muxNu;
  int *fuNu;
} MuxOutputs, *MoPtr;

/* Mux input/out pointer and number */
typedef struct Mux *MuxPtr;

struct Mux{
 MiPtr Mi; 
 MoPtr Mo;
 int m;
 int grp;
 MuxPtr next;
};

/* FU port number pointer */
typedef struct {
 MuxPtr *Fu_portNu;
} Fu;

/* FU's operation and number */
typedef struct Resource *ResourceList;

struct Resource{
 int op; 
 int nu;
 ResourceList next;
};

typedef struct OutputRdy *OutputRdyList;

struct OutputRdy{
 int outputRdyTime;
 int outputRdyNu;
 OutputRdyList next;
};


//----------------------------------------------
// Number of counters for each shared register
// for each counter, save its FSM enable time.
//
// SharedReg  counter enable1  enable 2 
//      1      2       8,      16
//      2      1       8
//      ...    ...     ...     ...
//----------------------------------------------
typedef struct {
  int cntNu;
  int *enableTime;
} FSM_outputs, *FSM_outputs_ptr;

/* Mux select enable time */
typedef struct muxSelTime *MuxSelList;

struct muxSelTime {
  int startTime; // MUX select enable time
  int selVal;    // MUX select value
  int inputNu;   // Number of MUX inputs
  int addMuxReg; // If Mux input I2 needs a register
  MuxSelList next;
};

/* Floating-point representation in C */
typedef union {
  int i;
  float f;
} int32_or_float;

/* Floating-point representation in C */
typedef union {
  long i;
  double f;
} int64_or_double;

/* 2-input node's constant in floating-point representation in C */
typedef union {
  int32_or_float sfp;
  int64_or_double dfp;
} float_or_double;

/* FU go enable time
typedef struct fuGoTime *FuGoList;
struct fuGoTime {
  int startTime; 
  FuGoList next;
}; */

/* Insert operation ID node into the list with PathLength priority */
List ListInsert(List head, int nodeID, int priority, enum priority type);

/* Delete operation ID node from the list with PathLength priority */
List ListDelete(List head, int nodeID);

/* Show the list contents */
void ListPrint(List head);

/* Calculate the path length from the operation ID node to SINK */
int PathLength(int nodeID);

/* Update DFG's priority member with the node's operation path length */
void SetPathLength(int path[]);

void SetNodePriority(int a0[], int p[], int nu);

void bubblesort(int a[], int p[], int st, int nu);
void priority_sort(int a0[], int a1[], int p[], int t1[], int nu);


/* Update DFG's priority member with the node's operation mobility */
void setMobility(int *ASAP_slots, int *ALAP_slots);

// Update ALAP slots of all the predecessors of node i
void Update_ALAP_Slots(int i, int *ASAP_slots, int *ALAP_slots, int flag);

// Update ASAP slots of all the successors of node i
void Update_ASAP_Slots(int i, int *ASAP_slots, int *ALAP_slots, int flag);

/* Save ASAP-scheduled time slots in the array ASAP_slots */
int ASAP(int *ASAP_slots, int *Xconstraint, int *Kconstraint);


/* Save ALAP-scheduled time slots in the array ALAP_slots */
int ALAP(int *ALAP_slots, int maxPortPair);

int ALAP1(int *ALAP_slots, int maxPortPair, int testGen);

/* List-based scheduling */
//int ListSchedule(int *ASAP_slots, int *ALAP_slots, int testGen);
int ListSchedule(int *ASAP_slots, int *ALAP_slots, int maxInputPortNu, int testGen);

void FDS(int *ASAP_slots, int *ALAP_slots);
void Schedule_Adjust(int *ASAP_slots, int *ALAP_slots, int nu, int slot);

float Cost(int *ASAP_slots, int *ALAP_slots);
int FuCost(int *ASAP_slots, int *ALAP_slots);
int PipeFuCost(int *ASAP_slots, int *ALAP_slots);

/* Create a list of nodes whose mobility is larger than 1 */
List CreateUnlockOpsList(List UnlockOpsList_head, int *ASAP_slots, int *ALAP_slots);

/* Maximum cumulative summation and its index */
int Max_Cum_Gain(int Gain[], int count);
int Max_Cum_Gain_Index(int Gain[], int count);

/* Check resource usage conflict */
int CheckConflict();

/* Check DFG */
void CheckDFG(); 

/* Reschedule the operations */
int Reschedule(int cycle);

/* Count the total number of FUs */
int CountResource();

/* True if there are available resources */
int ResourceAvailable();

/* Get available non-port resource number */
int GetResourceNu(int slot, int type);

/* Create non-port resource allocation table */
void CreateRAT();

/* Reset non-port resource allocation table */
void ResetRAT();

/* Update non-port resource allocation table */
void UpdateRAT(int nu, int op_type);

/* Dump non-port resource allocation table */
void DumpRAT();
int CountOpNode(int type);

/* Free all the malloced resource */
void FreeAll();

/* Create queue as resources */
void CreateResource();

/* Create simple DFG */
void CreateDFG();

/* Reset queue as resources */
void ResetResource();

/* Recycle scheduled resource */
void RecycleScheduledResource(int slot);

/* Print the resource numbers and their types, data path delay */
void PrintStats(FILE *fp, char *name, int latency, int resource, int delay);

/* Open statistics files  */
FILE *OpenStat(const char *priority, const char *filename);

/* Open .csv files for drawing */
void OpenCSV(FILE *csv[][5], const char *CircuitName, int P, int N, int px, int pk);

/* Close them */
void CloseCSV(FILE *csv[][5]);

/* Print resource ranges for each port number and partition */
void PrintScheduleStat(FILE *fp, int P, int N, int px, int pk, ScheduleStats SS[]);

/* Print Scheduled DFG */
void PrintScheduledDFG(int slot);

/* get register->mux or register->fu fanout */
void GetRegFanOut(int *Map, int k, int RF_PortNu);

/* Check different resources are assigned to the same operation 
 * at the same time slot*/
void CheckScheduling(int latency, int* ASAP_slots, int* ALAP_slots);

/* Register allocation with LEA */
int LeftEdgeRegisterBinding(int *Map);

/* Register allocation with Weighted Bipartite Matching */
int RegAlloc (int *Map, int delay);

/* Print variable list */
void VarListPrint(varList head);

/* Print variable life time */
void PrintVarLifeTime();

/* Create(L) */
varList CreateVarList(varList head);

/* FIRST(L, v) */
varList VarListFirst(varList head);

/* NEXT(L, v) */
//varList VarListNext(varList head, int varID);

varList VarListNext(varList head, varList currVar);

/* SORT(L) */
varList VarListInsert(varList head, int varID, int startTime, int endTime);

/* DELETE(L,v) */
varList VarListDelete(varList head, int varID);

/* Insert VarList table with varList */
void VarTableInsert(int regNu, varList currVar);

/* Store VarList for each shared register */
void CreateVarTable(int regNu);

/* Free table entry in VarTable */
void FreeVarTableEntry(const int entry);

/* Free the entire VarTable */
void FreeVarTable(const int SharedRegNu);

/* Interconnection allocation of MUXes 
 * Return total number of MUXes */
int InterconnectionAlloc (int *Map, int SharedRegNu);

/* Create Mux resource allocation table */
void CreateMuxRAT();

/* Insert allocated MUX into MuxList */
void MuxListInsert(int op_type, int fu_nu, int port_nu, MuxPtr nextMux);

/* Free MuxList */
void FreeMuxList();

/* Free Mux resource allocation table */
void FreeMuxRAT();

/* Print Mux resource allocation table */
void MuxRATPrint();

/* Find the MuxList and call Check MuxList */
MuxPtr CheckMuxAlloc(int *Map, int op_type, int fu_nu, int port_nu, 
                     int ri, int rj, int di, int dj, 
                     int constFlag, int *MuxCnt);

/* Check MuxList and decide if MUX should be allocated */
MuxPtr CheckMuxList(int *Map, MuxPtr head, 
                    int ri, int rj, int di, int dj, 
                    int fu_nu, int port_nu, 
                    int constFlag, int *MuxCnt);

/* Get parent Mux's pointer */
MuxPtr GetMuxID(MuxPtr head);

//MuxPtr FindMuxNu(MuxPtr head, int *currMuxNu);

/* Allocate a new MUX */
MuxPtr MuxAlloc(int *Map, MuxPtr parentMux, 
                int mux_i_r1, int mux_i_r2, 
                int mux_i_d1, int mux_i_d2, 
                float mux_i_c1, float mux_i_c2, 
                double mux_i_c1_d, double mux_i_c2_d, 
                int mux_i_m1, int mux_i_m2,
                int mux_i_f1, int mux_i_f2,
                int mux_o_r, int mux_o_m, 
                int mux_o_fu, int *MuxCnt);

/* Print all the functional unit connected to the shared register */
void ResourceListPrint(ResourceList head);

/* Create functional unit usage table */
void CreateFuRAT(int *Map, int SharedRegNu);

/* Free functional unit usage table */
void FreeFuRAT(int SharedRegNu);

/* Insert functional unit connected to the shared register */
ResourceList ResourceListInsert(ResourceList head, enum operation op, int resourceNu);

OutputRdyList OutputRdyListInsert(OutputRdyList head, int outputRdyTime, int *outputCnt);
void OutputRdyListPrint(OutputRdyList head) ;

void GenerateOutputAssign (FILE *vfp, char *ModuleName, const char *SignalName1, const char *SignalName2, const char *SignalName3);

/* Generate Backend */
void GenerateBackEnd(FILE *fp, char *scheduleName, const char *schedule,
                     int portNu, int scheduleDelay,
                     ScheduleStats SS[]);

/* Return number of Mux inputs */
int GenerateHDL(FILE *fp, int *Map, int *muxCnt, int SharedRegNu, char *ScheduleName, \
                int delay, ScheduleStats SS[]);

void GetTotalMuxInputs(int muxpCnt, int muxCnt, int SharedRegNu);
void GetTotalRegNum(int pipeline_depth, int SharedRegNu);
void GetHDLFanout(FILE *fp, int SharedRegNu);

int GetEndTime(int nodeID);

void MuxChainPartition();

/* Generate hw MUXes 
 * Return number of MUX inputs at Phase1 and 2*/
int GeneratePhase1Mux(FILE *vfp, FILE *fp, int *Map, int SharedRegNu);
int GeneratePhase2Mux(FILE *vfp, FILE *fp, int SharedRegNu, int *MuxCnt, int *MuxCntTable);

void GenerateRegMux(FILE *vpf, int regNu, int RF_PortNu,\
                    RF_portPtr RegFileTable, int dataWidth);

void GenerateRegMuxSel(FILE *vfp, int regNu, int RF_PortNu, 
                       RF_portPtr RegFileTable);

/* Generate hw functional unit */
void GenerateFU(FILE *vfp);
void GenerateFuncRegister(FILE *vfp);

/* Generate hw different registers */
int GenerateRegisters(FILE *vfp, FILE *fp, int *Map, char *ModuleName, int SharedRegNu);

/* Generate hw wires */
void GeneratePhase1Wire(FILE *vfp, FILE *fp, int *Map);
void GeneratePhase2Wire(FILE *vfp, int SharedRegNu);

/* Generate hw register file as delay unit */
void GenerateRegisterFile(FILE *vfp, int addrWidth, int dataWidth, 
                          int dataDepth, int cntVal, int regNu,
                          RF_portPtr RegFileTable, int DestNu);

void GenerateShiftRegister(FILE *vfp, FILE *fp, int addrWidth, int dataWidth, 
                          int dataDepth, int cntVal, int regNu, 
                          RF_portPtr RegFileTable, int DestNu);

int TotalRegFilePortNu(RF_portPtr RegFileTable, int DestNu);

/* Generate hw register */
void GenerateRegister(FILE *vfp, int dataWidth, int regNu);
void GenerateRegisterC(FILE *vfp, int dataWidth, int regNu);
//void GenerateRegisterV(FILE *vfp, int dataWidth, int regNu);

/* Generate symbolic FSM */
void GenerateSymbolicFSM (FILE *vfp, int *Map, int SharedRegNu, 
                          int *MuxCnt, int *MuxCntTable, int delay,
                          char *ScheduleName);

/* Generate symbolic Mux select */
void GenerateSymbolicMuxSel1(int i, int *Map);
void GenerateSymbolicMuxSel2(int i, int *Map, int *MuxCntTable);
void GenerateSymbolicMuxSel(MuxPtr p, int sel, int nodeNu);

void PrintMuxSelList(int *MuxCnt);

FSM_outputs_ptr *GenerateMuxSel1(FILE *vfp, int Phase1MuxCnt, int *MuxCnt);
FSM_outputs_ptr *GenerateMuxSel2(FILE *vfp, FSM_outputs_ptr *MuxSelCtlTable, 
                                 int Phase1MuxCnt, int *MuxCnt, char *ScheduleName);


/* */
FSM_outputs_ptr *CreateCntEnableTable(int SharedRegNu);

/* */
FSM_outputs_ptr *UpdateCntEnableTable (FSM_outputs_ptr *ptr, int regNu, 
                                       int multiple, int latency);

/* */
void MuxSelListInsert(int muxNu, int startTime, int selVal, int inputNu);

/* */
void CreateMuxSelTable(int MuxCnt);

/* */
void FreeMuxSelTable(int MuxCnt);

void FreeTables(int SharedRegNu);

void RegFileTableCheckInsert(RF_portPtr RegFileTable, 
                            int RF_PortNu, 
                            int curr_child_ID);

void RegFileTableInsert(RF_portPtr RegFileTable, 
                       int RF_PortNu,
                       int curr_child_ID);

int AddRegFilePortNu (RF_portPtr RegFileTable, int curr_child_ID, int DestNu);

void AddMuxRegister(int *Map);
void CheckMuxRegister(enum operation op_type, int fu_nu, int port_nu, int sharedReg, int mux_nu);

//
int GetRegFilePortNu(int nodeID, int childID);

//
int GenerateRegFilePortMux(FILE *vfp, int *Map, int nodeID, int childID);

void PrintRegFileTable(RF_portPtr RegFileTable, int DestNu);

void FreeRegFileTable (int nodeID);

/* */
void FreeCntEnableTable(FSM_outputs_ptr *ptr, int SharedRegNu);

/* Generate symbolic Register write enable */
FSM_outputs_ptr *CollectRegWen(FILE *vfp, int SharedRegNu);
FSM_outputs_ptr *CollectRegFileRen(FILE *vfp, int SharedRegNu);

/* Register File read enables in FSM */
void GenerateRegFileRen(FILE *vfp, int *Map, int regNu, int time);

void GenerateDefaultRegFileRen(FILE *vfp, int *Map);

/* */
void GenerateCounter (FILE *vfp, int sum, char *instanceName, int regNu, 
                      char instanceNuIdx, int flag);

void GenerateOutputsAssign (FILE *vfp, char *ModuleName);

/* */
void GenerateSymbolicRegisterWen(FILE *vfp, int SharedRegNu);

/* Generate symbolic FU go */
void GenerateSymbolicFUGo(FILE *vfp);

//void GenerateControlWires(FILE *vfp, FSM_outputs_ptr *MuxSelCtlTable, \
                          FSM_outputs_ptr *RegWenCtlTable, int *MuxCnt,\
                          int SharedRegNu);
void GenerateControlWires(FILE *vfp, int *MuxCnt, int SharedRegNu);

void GenerateControlFSM(FILE *vfp);

void GenerateControlEnable(FILE *vfp, 
                           FSM_outputs_ptr *MuxSelCtlTable, 
                           FSM_outputs_ptr *RegWenCtlTable, 
                           FSM_outputs_ptr *RegRenCtlTable, 
                           int *MuxCnt, int SharedRegNu,
                           int *Map);


void GenerateEnableDefault(FILE *vfp, char *instanceName, 
                           FSM_outputs_ptr *MuxSelCtlTable, 
                           int compNu);

void FindSameEnableTime(FILE *vfp, int nu, int compNu, char *instanceName,
                        FSM_outputs_ptr *MuxSelCtlTable, 
                        FSM_outputs_ptr *MuxSelCtlMarkTable, 
                        int *Map, int time);

/* Generate FU go */
void GenerateFUGo(FILE *vfp);

/* Generate hw control MUXes */
void GeneratePhase1CWire(FILE *vfp, int *Map);
void GeneratePhase2CWire(FILE *vfp, int SharedRegNu);

/* Generate hw control MUXes */
void GeneratePhase1CMux(FILE *vfp, int *Map, int *MuxCnt);
void GeneratePhase2CMux(FILE *vfp, int SharedRegNu, int *MuxCnt, int *MuxCntTable);

/* Generate hw control DEMUXes */
void GeneratePhase1DeMux(FILE *vfp, int *Map);

/* Operation symbol table look-up */
char *opSymTable(enum operation op);
char *opTypeTable(enum operation op);

/* Arithmetic operator */
char opTable(int op);

/* Input ports table look-up */
char *PortSymTable(int op);

/* */
char *CounterSignalSymTable(char *instanceName, int signalNu);

/* */
char *ROMSignalSymTable(int signalNu);

/* Integer log2
 * 8 -> 3
 * 9 -> 4 */
int IntLog2(int number);

/* Integer log2 
 * 8 -> 4
 * 9 -> 4 */
int IntLog2a(int number);

int FloatingPointConvert(float num);
long DoubleFloatingPointConvert(double num);

char *GetFileName(char *fileName, char *fileType, char *fileExt, char *ScheduleName);

/* */
void CopyFile(char *srcFileName, FILE *destFileName);

void GenerateDoFile(char *ScheduleName);

/* */
void GenerateModulePort(FILE *vfp, char *ModuleName);

#ifdef DATA_PATH_ONLY
void GenerateRegEnablePort(int regNu);
void GenerateRegFileEnablePort(int regNu, RF_portPtr RegFileTable, int DestNu);
int GenerateMuxSelPort(int SharedRegNu);
#endif

/* */
void GenerateStallControl(FILE *vfp, int delay);


/* */
void GenerateModuleSignals(FILE *vfp, char *ModuleName);

void GenerateCounters(FILE *vfp, char *ModuleName, 
                      const char *SignalName1,
                      const char *SignalName2,
                      const char *SignalName3);

/* */
void GenerateTestBench(char *ModuleName, char *ScheduleName, int pipeline_depth);

/* */
void GenerateInstance(FILE *vfp, char *ModuleName);

/* */
int GenerateTestData(int TestCount);

/* */
void ResetStimulus(FILE *vfp);

/* */
void CollectStimulusInfo(int slot, int op_type, int resourceNu);

/* */
int GenerateStimulus(FILE *vfp, int TestCount);

/* */
void GenerateSetUp(FILE *vfp, FILE *cfp, char *ModuleName, char *ScheduleName, int pipeline_depth);

/* */
void GenerateDebugData(FILE *vfp, FILE *cfp, char *ModuleName, int testCount); 
void GenerateHWDebugData(FILE *vfp, char *ModuleName, int testCount, int *FUGoTimeIdx);
void GenerateSWDebugData(FILE *cfp, char *ModuleName, int testCount, int *FUGoTimeIdx);

/* */
void DumpOutputValues(FILE *vfp, char *ModuleName);

/* */
void FreeStimTable (int timeSlotNu);

/* */
int UseROM(enum operation opType);

int DataPathWidth(enum operation op_type, int io);

int GetNodeSrcNu(enum operation op_type);

extern const char *CircuitName;
//extern char vfilename[50];
extern opNode **DFG, **DFG_current;
extern queue **RQ;
extern queue **PQ;
extern int *ASAP_slots, *ALAP_slots;
extern int *PortPriority;
extern int *SortedPortPriority;
extern int *NodePriority;
extern int *PortAssignment;
extern int **RAT;
extern int **PRAT;
extern Fu **MuxRAT;
extern ResourceList *FuRAT;
extern SRAT_FuType_Ptr *SRAT;
//extern ResourceList *StimTable;
extern int *StimSequenceTable;
extern varList *VarTable;
extern MuxSelList *MuxSelTable;

extern portStringPtr PortNameList;
extern portStringPtr PortDeclList;

extern int PORT_NU;
extern int MAX_PORT_NU;
extern int NODE_NU;
extern int minII;

extern int RATE[OP_NU];
extern int LATENCY[OP_NU];
extern int PORTS[OP_NU];
extern int MAX_PORTS[OP_NU];
extern int FU[OP_NU];
extern struct conflict_table table[OP_NU]; 

// Register fan-out table
extern FanoutR_ptr RegFanout;

// Functional Unit fan-out table
extern int **FuFanout;

// Registered input port fan-out table
extern int *PortFanout;

// WBM register allocation failure
extern int WBM_FAILED;

// branch/bound search 
extern double ENUM_NU;
extern double ENUM_SZ;
extern int min_mux_fanin, max_mux_fanin,
           min_mux_input, max_mux_input,
           min_muxp_input, max_muxp_input,
           min_muxr_input, max_muxr_input;
extern int min_mux_fanout, max_mux_fanout;
extern int min_reg_nu, max_reg_nu;
extern int min_latency, max_latency;

// CSV file

#define REG 0 
#define MUX 1 
#define DLY 2 
#define FAN 3 
#define NV 0 

extern char stat[100];
extern FILE *csv[3][5];

extern  CircuitStats CS;
extern  CircuitStatsPtr CSP;

char * gnu_getcwd ();
void * xmalloc (size_t size);

#endif /* SCHEDULE_ */
