#ifndef STATE_
#define STATE_

#include <stdio.h>

enum {
  NPREF   = 2,          /* number of prefix words */
  NHASH   = 6,          /* size of state hash table array */
  MAXGEN  = 10000       /* maximum words generated */
};

typedef struct Node Node;

// id = -1 ... -N  for leaf node (VARIABLE)
// id = 0 for leaf node (NUMBER)
// id >= N for non-leaf node
struct Node {
  int    v;     /* node visited ? */
  int    id;    /* node id */ 
  char   op;    /* operator */
  Node** child; /* node's children */
  char   type;  /* leaf type */
  union {
    double num;   /* constant */
    char *s;    /* identifier  */
  } value;
};

typedef struct State State;
typedef struct Suffix Suffix;

// State for leaf node only
struct State {
  char   *s;            /* id */
  int    id;            /* id value */
  Node   *ptr;          /* pointer to a node */
  State  *next;         /* next in hash table */ 
};

State *statetab[NHASH];

/* hash: compute hash value for an array of NPREF strings */
unsigned int hash (char *s);

/* lookup: search for string; create if requested. */
/* returns pointer if present or created; NULL if not. */
/* creation doesn't strdup so strings mustn't change later. */
//State* lookup(char *s, int create);
Node* LookupToken(char *s);
State* AddToken(char *s, Node *node);

#endif /* STATE_ */
