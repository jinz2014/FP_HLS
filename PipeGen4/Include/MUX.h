#ifndef MUX_CHAIN_OP_
#define MUX_CHAIN_OP_

#ifdef ADD_MUX_REG_DIV
  #define MUX_DIV 1
#else
  #define MUX_DIV 0 
#endif

#ifdef ADD_MUX_REG_ADD
  #define MUX_ADD 2
#else
  #define MUX_ADD 0 
#endif

#ifdef ADD_MUX_REG_MUL
  #define MUX_MUL 4
#else
  #define MUX_MUL 0 
#endif

#ifdef ADD_MUX_REG_SUB
  #define MUX_SUB 8
#else
  #define MUX_SUB 0 
#endif

#define MUX_OP (MUX_DIV + MUX_ADD + MUX_MUL + MUX_SUB)

typedef struct {
  int use; 
  unsigned char rfp[10]; // regfile port
} reg_st;

#endif 
