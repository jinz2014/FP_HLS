#ifndef _ExprParser
#define _ExprParse

//#define DEBUG

#include "State.h"
void CutTree(Node *root, Node *m);
void PrintTree(Node *m);
//int MarkLeaf(Node *m);
void MarkLeaf(Node *m, int *leaf_num, int *port_num);
int MarkNode(Node *m, int p);

int visit(Node *m, int pid, int destNu, int dump);
void GenerateLeafNodes(Node *m, Node *root);
void GenerateNonLeafNodes(Node *m, Node *root, int nid);
void GenerateExprFunc(Node *m);
int SearchLeaf(Node *m);
void SearchNode(Node *node, Node *m);

void GenerateDotFile(Node *root);
void DrawDFG(Node *root, Node *m);
void draw(Node *node, Node *m);
void AssignLeafRank(Node *m);
void ResetVisitedNode(Node *m);

void FreeHashTable();

void eval_exp ();
void eval_exp2(Node **node);
void eval_exp3(Node **node);
//void eval_exp4(Node *node);
void eval_exp5(Node **node);
void eval_exp6(Node **node, char op);
void eval_exp7(Node **node);
void atom(Node **node, char op);

void unary(char o, double *r);
void serror(int error);
void get_token();
void get_variable();
int isdelim(char c);

void myprintf(const char* format, ... );


// globals
extern char *expr;
extern char *name;
extern FILE *fp; // DFG 
extern FILE *cfp; // C model
extern FILE *gfp; // graphical DFG

#endif

