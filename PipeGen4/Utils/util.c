#include "Schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Translate numeric operation to a general operation type
//-----------------------------------------------------------------------------
char *opTypeTable(enum operation op) {
  char *opType;
  switch (op) {
   // port node
    case nop  : opType = "port"; break;
    case nopc : opType = "cport"; break;

   // single-precision operation
    case add : opType = "arithmetic"; break;
    case mul : opType = "arithmetic"; break;
    //case addx: opType = "arithmetic"; break;
    //case mulx: opType = "arithmetic"; break;
    case div_: opType = "arithmetic"; break;
    case sub : opType = "arithmetic"; break;
    case mx  : opType = "select"; break;
    case max : opType = "arithmetic"; break;
    case rom : opType = "memory"; break;
    case lt  : opType = "compare"; break;
    case gt  : opType = "compare"; break;

   // double-precision operation
    case addd : opType = "arithmetic"; break;
    case muld : opType = "arithmetic"; break;
    case divd : opType = "arithmetic"; break;
    case subd : opType = "arithmetic"; break;
    case mxd  : opType = "select"; break;
    case maxd : opType = "arithmetic"; break;
    case romd : opType = "memory"; break;
    case ltd  : opType = "compare"; break;
    case gtd  : opType = "compare"; break;

   // single to double precision conversion
    case tod  : opType = "conversion"; break;
    case tof  : opType = "conversion"; break;
    default: { 
      opType = "unknown";
      myprintf("Unknown op %d\n in TypeTable", op); 
    }
  }
  return opType;
}

//-----------------------------------------------------------------------------
// Translate numeric operation to a less general operation type
//-----------------------------------------------------------------------------
char GetOpSym(enum operation op) {
  char operator;
  switch (op) {
    case add:  operator= '+'; break;
    //case addx: operator= '+'; break;
    case addd: operator= '+'; break;
    case mul:  operator= '*'; break;
    //case mulx: operator= '*'; break;
    case muld: operator= '*'; break;
    case sub:  operator= '-'; break;
    case subd: operator= '-'; break;
    case div_: operator= '/'; break;
    case divd: operator= '/'; break;
    case gt:   operator= '>'; break;
    case gtd:  operator= '>'; break;
    case lt:   operator= '<'; break;
    case ltd:  operator= '<'; break;
    case mx:   operator= 'm'; break;
    case mxd:  operator= 'm'; break;
    case max:  operator= 'x'; break;
    case maxd: operator= 'x'; break;
    case rom:  operator= 'r'; break;
    case romd: operator= 'r'; break;
    case tod:  operator= '2'; break;
    case tof:  operator= '5'; break;
    case nop:  operator= 'p'; break;
    case nopc: operator= 'p'; break;
    default: { 
      printf("Unknown op %d\n in opTable", op); 
      operator = '?';
      exit(1);
    }
  }
  return operator;
}


//-----------------------------------------------------------------------------
// Translate numeric operation to the corresponding operation name
//-----------------------------------------------------------------------------

char *opSymTable(enum operation op) {
  char *opSym;
  switch (op) {
   // port node
    case nop  : opSym = "port"; break;
    case nopc : opSym = "cport"; break;
   // single-precision operation
    case add : opSym = "fadd"; break;
    //case addx: opSym = "faddx"; break;
    case mul : opSym = "fmul"; break;
    //case mulx: opSym = "fmulx"; break;
    case div_: opSym = "fdiv"; break;
    case sub : opSym = "fsub"; break;
    case mx  : opSym = "fmux"; break;
    case max : opSym = "fmax"; break;
    case rom : opSym = "rom"; break;
    case lt  : opSym = "flt"; break;
    case gt  : opSym = "fgt"; break;
   // double-precision operation
    case addd : opSym = "faddd"; break;
    case muld : opSym = "fmuld"; break;
    case divd : opSym = "fdivd"; break;
    case subd : opSym = "fsubd"; break;
    case mxd  : opSym = "fmuxd"; break;
    case maxd : opSym = "fmaxd"; break;
    case romd : opSym = "romd"; break;
    case ltd  : opSym = "fltd"; break;
    case gtd  : opSym = "fgtd"; break;
   // single to double precision conversion
    case tod  : opSym = "ftod"; break;
   // double to single precision conversion
    case tof  : opSym = "dtof"; break;
    default: { 
      opSym = "unknown";
      myprintf("Unknown op %d\n in SymTable", op); 
    }
  }
  return opSym;
}

//=========================================================
// Return a node's number of inputs
// 
// Note if the node struct has a source_number
// field, then we don't need to specify the number
// of source inputs here again !
//=========================================================
int GetNodeSrcNu(enum operation op_type) {
  switch (op_type) {

    // for mrbay's app only
    case rom:
    case romd: return 4;

    // 2-1 mux only
    case mx:
    case mxd:  return 3;

    // single to double
    case tod:  
    case tof:  return 1;
    //
    default:   return 2;
  }
}

//=========================================================
// FU i/o wire connection data width 
// 1-output 0-input 
//=========================================================
int DataPathWidth(enum operation op_type, int io) {
    switch (op_type) {
      case add: 
      //case addx: 
      case sub: 
      case mul: 
      //case mulx: 
      case div_: 
      case mx: 
      case max: 
      case nop: return 32;
      case nopc:return 64;
      case rom: return io ? 32 : 1;
      case gt: 
      case lt:  return io ? 1 : 32;

      case addd: 
      case subd: 
      case muld: 
      case divd: 
      case mxd: 
      case maxd: return 64;
      case tod:  return io ? 64 : 32;          
      case tof:  return io ? 32 : 64;          
      case romd: return io ? 64 : 1;
      case gtd: 
      case ltd:  return io ? 1 : 64;

      default: 
      { 
        printf("unknown FU op_type value %d", op_type); 
        exit(1);
      };
    }
}


// return true if the opType exists in the DFG
// Name is a little confusing
int UseROM(enum operation opType) {
  enum operation op;
  for (op = add; op < add + OP_NU; op++) {
    if (opType == op) return 1;
  }
  return 0;
}
//========================================================
// Application specific ports
//
// App: mrbay
// The val of each node must match the spec of mrbay.txt 
//========================================================
char *GetCPortName(int val) {
  char *cportSym;
  switch (val) {
    case 1 : cportSym = "Norm"; break;
    case 2 : cportSym = "II1"; break; 
    case 3 : cportSym = "II2"; break;
    case 4 : cportSym = "II3"; break;
    case 5 : cportSym = "II4"; break;

    case  6 : cportSym = "PL_AA"; break;
    case  7 : cportSym = "PL_AC"; break;
    case  8 : cportSym = "PL_AG"; break;
    case  9 : cportSym = "PL_AT"; break;
    case 10 : cportSym = "PR_AA"; break;
    case 11 : cportSym = "PR_AC"; break;
    case 12 : cportSym = "PR_AG"; break;
    case 13 : cportSym = "PR_AT"; break;

    case 14 : cportSym = "PL_CA"; break;
    case 15 : cportSym = "PL_CC"; break;
    case 16 : cportSym = "PL_CG"; break;
    case 17 : cportSym = "PL_CT"; break;
    case 18 : cportSym = "PR_CA"; break;
    case 19 : cportSym = "PR_CC"; break;
    case 20 : cportSym = "PR_CG"; break;
    case 21 : cportSym = "PR_CT"; break;

    case 22 : cportSym = "PL_GA"; break;
    case 23 : cportSym = "PL_GC"; break;
    case 24 : cportSym = "PL_GG"; break;
    case 25 : cportSym = "PL_GT"; break;
    case 26 : cportSym = "PR_GA"; break;
    case 27 : cportSym = "PR_GC"; break;
    case 28 : cportSym = "PR_GG"; break;
    case 29 : cportSym = "PR_GT"; break;

    case 30 : cportSym = "PL_TA"; break;
    case 31 : cportSym = "PL_TC"; break;
    case 32 : cportSym = "PL_TG"; break;
    case 33 : cportSym = "PL_TT"; break;
    case 34 : cportSym = "PR_TA"; break;
    case 35 : cportSym = "PR_TC"; break;
    case 36 : cportSym = "PR_TG"; break;
    case 37 : cportSym = "PR_TT"; break;

    default: {
      printf("val = %d Error in portSym\n", val);
      cportSym = "Error";
      //exit(-1);
    }
  }
  return cportSym;
}

/* app-specific
char *GetOutPortName(int val) {
#ifdef MRBAY
  char *OutPortSym;
  switch (val) {
    case 0 : OutPortSym = "clP0"; break;
    case 1 : OutPortSym = "clP1"; break; 
    case 2 : OutPortSym = "clP2"; break;
    case 3 : OutPortSym = "clP3"; break;
    case 4 : OutPortSym = "scP"; break;
    case 5 : OutPortSym = "lnScaler"; break;
    case 6 : OutPortSym = "lnL"; break;
    default: {
      printf("val = %d Error in OutPortSym\n", val);
      OutPortSym = "Error";
    }
  }
#else
  char *OutPortSym = (char *) malloc (sizeof(char) * 10);
  sprintf(OutPortSym, "clP%0d", val);
#endif

  return OutPortSym;
}
*/

void myprintf(const char* format, ... ) {
#ifdef DEBUG
  va_list args;
  va_start( args, format );
  vfprintf( stdout, format, args );
  va_end( args );
#endif
}

void myfprintf(FILE *fp, const char* format, ... ) {
#ifdef DEBUG
  va_list args;
  va_start( args, format );
  vfprintf( fp, format, args );
  va_end( args );
#endif
}

void PrintArray(int *a, int n) {
  int i;
  for (i = 0; i < n; i++) myprintf("%3d ", a[i]);
  myprintf("\n");
}


void PrintDFG(int *ASAP_slots, int *ALAP_slots) {
  int i;
  for (i = 0; i < NODE_NU; i++) {
    fprintf(stderr, "Node %3d <- slot b: %3d e: %3d <%3d, %3d> type %5s resource %3d %s\n", 
        i, DFG[i]->opScheduledSlot, DFG[i]->opResultDoneSlot, 
        ASAP_slots[i], ALAP_slots[i],
        opSymTable(DFG[i]->op), DFG[i]->opResourceNu, DFG[i]->opName);
  }
}

inline myceil(const int x, const int y) {
  if (x <= y) 
    return 1;
  else
    return (x / y + ((x % y) ? 1 : 0));
}


//----------------------------------------------------
// ceil(log2(number))
//----------------------------------------------------
int IntLog2(int number) {
  
   double fval = log((double) number) / log(2);
   double ival = (int) fval; 
   return ((int) ((fval - ival) ? ival + 1 : ival)); 
}

//
int IntLog2a(int number) {
  
   double fval = log((double) number) / log(2);
   double ival = (int) fval; 
   return ((int) ival + 1);
}

int MyMax(int *x, int *y) {
  if (*x > *y) *y = *x;
}

int MyMin(int *x, int *y) {
  if (*x < *y) *y = *x;
}


void *
xmalloc (size_t size)
{
  register void *value = malloc (size);
  if (value == NULL) {
    fprintf(stderr, "Virtual memory exhausted");
    exit(1);
  }
  return value;
}

char *
gnu_getcwd ()
{
  size_t size = 100;

  while (1)
  {
    char *buffer = (char *) xmalloc (size);
    if (getcwd (buffer, size) == buffer)
      return buffer;
    free (buffer);
    if (errno != ERANGE)
      return 0;
    size *= 2;
  }
}

