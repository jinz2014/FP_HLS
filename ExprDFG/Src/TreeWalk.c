/* 
 * Parse a simple expression (variable, number and delimiter)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <assert.h> 
#include "ExprParser.h"
#include "TreeWalk.h"
#include "mystack.h"

#define OTHERS    0
#define VARIABLE  1
#define NUMBER    2
#define DELIMITER 3

void PrintRawTree(char *s, Node *p);

char token[80];
char token_type;

//========================================================================
// Parser entry point
//========================================================================
void eval_exp()
{
  int i;
  Stack S;

  // Save the pointer to the expression being parsed
  char *s = expr;

  get_token();
  if(!*token) {
    serror(2);
    return;
  }

  Node *root = NULL;

  // evalute expression
  eval_exp2(&root);

  // recursively mark Tree 
  int leaf_num = 0;
  int port_num = -1;

  MarkLeaf(root, &leaf_num, &port_num);
  myprintf("DFG has %d leaves\n", leaf_num);

  int nonleaf_num = MarkNode(root, leaf_num);
  myprintf("DFG has %d nonleaves\n", nonleaf_num - leaf_num);

  printf("preorder arithmetic tree walk\n");
  preorder_tree_walk(&S, root);

  printf("inorder arithmetic tree walk\n");
  inorder_tree_walk(&S, root);

  ResetVisitedNode(root);
  printf("postorder arithmetic tree walk\n");
  postorder_tree_walk(&S, root);

  char pre[500];
  *pre = '\0';
  PrintRawTree(pre, root);
  *pre = '\0';
  PrintComputedTree(pre, root);

  // recursively remove nodes
  CutTree(root, root);
  FreeHashTable();
}

void preorder_tree_walk(Stack *S, Node *root) {
  Node *p;
  init(S);
  push(S, root);
  while (empty(S) != 1) {
    p = pop(S);
    if (p != NULL) {

      printf("visit node ");
      if (p->type == VARIABLE)
        printf("%s\n", p->value.s);
      else if (p->type == NUMBER)
        printf("%f\n", p->value.num);
      else
        printf("%c(%d)\n", p->op, p->id);

      if (p->child != NULL) {
        push(S, p->child[1]);
        push(S, p->child[0]);
      }
    }
  }
}

void inorder_tree_walk(Stack *S, Node *root) {

  Node *p = root;
  init(S);
  push(S, p);

  while (1) {
    if (p->child !=NULL) {
      p = p->child[0];
      push(S, p);
    } 
    else {
      if (empty(S) != 1) {
        p = pop(S);
        printf("visit node ");
        if (p->type == VARIABLE)
          printf("%s\n", p->value.s);
        else if (p->type == NUMBER)
          printf("%f\n", p->value.num);
        else
          printf("%c(%d)\n", p->op, p->id);

        if (p->child != NULL) {
          p = p->child[1];
          push(S, p);
        }
      } else {
        break;
      }
    }
  } 
}

/* Reference Wiki */
void postorder_tree_walk(Stack *S, Node *root) {

  Node *p = root;
  init(S);
  push(S, p);

  while (empty(S) != 1) {

    p = peek(S);

    if (p->child !=NULL && !p->child[0]->v) {
      push(S, p->child[0]);
    }
    else if (p->child != NULL && !p->child[1]->v) {
      push(S, p->child[1]);
    }
    else {
      p = pop(S);
      p->v = 1;
      printf("visit node ");
      if (p->type == VARIABLE)
        printf("%s\n", p->value.s);
      else if (p->type == NUMBER)
        printf("%f\n", p->value.num);
      else
        printf("%c(%d)\n", p->op, p->id);
    }
  } 
}


//============================================================================
// Data Structure pp218 
// 
// A tree traversal in reverse inorder, that has an additional parm "s", whose
// value is the string of characters to be printed as a prefix at the left
// end of every line of the tree. Initially s is an empty string, but as we
// go deeper in the tree, blanks are added to it; these blanks are removed as
// we go back up toward the root.
// 
// procedure PRINTTREE(pre, t)
//   if t != nil then
//      PRINTTREE(pre || " ", RIGHT(t))
//      print pre || INFO(t)
//      PRINTTREE(pre || " ", LEFT(t))
//
// || is string concatenation.
//
// Notice how blacks are added and removed when going deeper or shallower.
//
// The number of spaces (maybe not just one space) also needs to be 
// adjusted for a better look.
//============================================================================
void PrintRawTree(char *s, Node *p) {

  char *pre;
  int sl;

  if (p->child != NULL) {
    sl = strlen(s) + 4; // n-1: 3 spaces
    pre = (char *) malloc (sizeof(char) * sl);
    strcpy(pre, s);
    PrintRawTree(strcat(pre, "   "), p->child[1]); 
    free(pre);
  }


  printf("%s", s);
  if (p->type == VARIABLE) {
    printf("%s\n", p->value.s);
  }
  else if (p->type == NUMBER) {
    printf("%f\n", p->value.num);
  }
  else {
    printf("%c\n", p->op);
  }

  if (p->child != NULL) {
    pre = (char *) malloc (sizeof(char) * sl);
    strcpy(pre, s);
    PrintRawTree(strcat(pre, "   "), p->child[0]); 
    free(pre);
  }
}

void PrintComputedTree(char *s, Node *p) {
  char *pre;
  char *sd;
  int sl;

  if (p->child != NULL) {
    sl = strlen(s) + 4; // n-1: 3 spaces
    pre = (char *) malloc (sizeof(char) * sl);
    strcpy(pre, s);
    if (p->child[1]->child == NULL)
      PrintComputedTree(strcat(pre, " --"), p->child[1]); 
    else
      PrintComputedTree(strcat(pre, "|  "), p->child[1]); 

    free(pre);
  }

  /*
  sd = strdup(s);
  while(*sd != '\0') {
    if ((sd = strchr(sd, '|')) != NULL)
      *sd  = ' ';
    else
      break;
  }
  */

  if (p->type == VARIABLE) {
    printf("%s", s);
    printf("%s\n", p->value.s);
  }
  else if (p->type == NUMBER) {
    printf("%s", s);
    printf("%f\n", p->value.num);
  }
  else {
    printf("%s", s);
    printf("%c\n", p->op);
  }

  //free(sd);

  if (p->child != NULL) {
    pre = (char *) malloc (sizeof(char) * sl);
    strcpy(pre, s);
    if (p->child[0]->child == NULL)
      PrintComputedTree(strcat(pre, " --"), p->child[0]); 
    else
      PrintComputedTree(strcat(pre, "|  "), p->child[0]); 
    free(pre);
  }
}

 
//========================================================================
// Construct a (add/sub) node whose left child points to the left child node
// and right child points to the right child node.
//
// Return a pointer to the current (add/sub) node
//========================================================================
void eval_exp2(Node **m)
{
  char op;
  eval_exp3(m);
  while ((op = *token) == '+' || op == '-') {
    Node *cn = (Node *) malloc (sizeof(Node));
    cn->op = op;
    cn->type = DELIMITER;
    cn->child = (Node **) malloc (sizeof(Node *) * 2);
    cn->child[0] = *m; 

    get_token();
    eval_exp3(m);
    cn->child[1] = *m;

    /* common subexpression test
    if (cn->op == '-')
      printf("%d %c %p %p\n", 
          cn->id, cn->op, cn->child[0], cn->child[1]);
     */
    *m = cn; 
  }
}

     
//========================================================================
// Construct a (mul/div) node whose left child points to the left child node
// and right child points to the right child node.
//
// Return a pointer to the current (mul/div) node
//========================================================================
void eval_exp3(Node **m)
{
  char op;
  eval_exp5(m);
  while ((op = *token) == '*' || op == '/') {
    Node *cn = (Node *) malloc (sizeof(Node));
    cn->op = op;
    cn->type = DELIMITER;
    cn->child = (Node **) malloc (sizeof(Node *) * 2);
    cn->child[0] = *m; 

    get_token();
    eval_exp5(m);
    cn->child[1] = *m;

    *m = cn; // left child 
  }
}

/* Evaluate a unary + or - */
void eval_exp5(Node **m)
{
  char op = 0;
  if ((token_type == DELIMITER) && (*token == '+' || *token == '-')) {
    op = *token;
    get_token();
  }
  eval_exp6(m, op);
}

/* Process a parenthesized expression */
void eval_exp6(Node **node, char op)
{
  // Assume minus sign exists only before a constant 
  if (*token == '(') {
    get_token();
    eval_exp2(node);
    if (*token != ')') serror(1);
    get_token();
  }
  else 
    atom(node, op);
}

/* Get the actual value of a variable or number */
void atom(Node **node, char op)
{
  int addNode = 1;
  Node *ptr;

  // Don't add token if it is already in the hash table
  // For constant, use separate nodes even if their values are the same
  if (token_type == VARIABLE) {
    if ((ptr = LookupToken(token)) != NULL)
      addNode = 0; 
  }

  if (addNode) { 
    Node *n  = (Node *) malloc (sizeof(Node));
    *node    = n;
    n->v     = 0;
    n->op    = 'p';
    n->type  = token_type;
    n->child = NULL;

    if (n->type == VARIABLE) {

      AddToken(token, *node); // Add variable token to the hash table

      n->value.s = (char *) malloc (sizeof(char) * (strlen(token) + 1));
      strcpy(n->value.s, token); 
      //get_token();
    } 
    else if (n->type == NUMBER) {
      if (op == '-') 
        n->value.num =  -atof(token);
      else
        n->value.num =  atof(token);
      //get_token();
    } 
    else {
      serror(0);
    }
  } else {
    // return the pointer to an already parsed node 
    *node = ptr;
  }
  // get next token anyway
  get_token();
}

/* Display a syntax error */
void serror(int error) 
{
  static char *e[] = {
    "syntax error",
    "unbalanced parentheses",
    "no expression present"
  };
  myprintf("%s\n", e[error]);
}

/* Return true if c is a delimter */
int isdelim(char c)
{
  if (strchr(" +-/*%^=()", c) || c == 9 || /*c == 'r' ||*/ c == 0)
    return 1;
  return 0;
}

/* Return the next token */
void get_token(void) 
{
  char *temp;

  token_type = OTHERS;
  temp = token;
  *temp = '\0';

  /* at end of expression */
  if (!*expr) return;

  /* skip over whitespaces */
  while(isspace(*expr)) ++expr;

  if (strchr("+-*/%^=()", *expr)) {
    token_type = DELIMITER;
    /* advance to next char */
    *temp++ = *expr++;
  }
  else if (isalpha(*expr)) {
    while(!isdelim(*expr)) *temp++ = *expr++;
    token_type = VARIABLE;
  }
  else if (isdigit(*expr)) {
    while(!isdelim(*expr)) *temp++ = *expr++;
    token_type = NUMBER;
  }
    *temp = '\0';
    myprintf("The next token is %s\n", token);
}

// Find the variable token only. 
void get_variable(void) 
{
  char *temp;

  temp = token;
  *temp = '\0';

  if (isalpha(*expr))
    while(!isdelim(*expr)) *temp++ = *expr++;
  else 
    *temp++ = *expr++;

  *temp = '\0';
}

//============================================================
// Helpers
//============================================================

char* MmOp(char op) {
  char* s = (char *) malloc (sizeof(char) * 10);
  switch (op) {
    case 'p': strcpy(s, "nop"); break;
    case '+': strcpy(s, "add"); break;
    case '-': strcpy(s, "sub"); break;
    case '*': strcpy(s, "mul"); break;
    case '/': strcpy(s, "div_"); break;
    default: { 
      myprintf("Unknown op %c\n", op); 
      exit(1);
    }
  }
  return s;
}

/* hash: compute hash value for the token string */
unsigned int hash (char *s) {
  unsigned int h;
  unsigned char *p;
  const int MULTIPLIER = 37;
  
  h = 0;
  for (p = (unsigned char *) s; *p != '\0'; p++)
    h = MULTIPLIER * h + *p;
  return h % NHASH;
}

/* lookup: search for id; create if requested. */
/* returns pointer if present or created; NULL if not. */
/* creation doesn't strdup so strings mustn't change later. */
Node* LookupToken(char *s) {
  int h;
  State *sp;
  h = hash(s);

  for (sp = statetab[h]; sp != NULL; sp = sp -> next) {
    if (strcmp(s, sp->s) == 0)
      return sp->ptr;
  }
  return NULL;
}

void FreeHashTable() {
  int h;
  State *sp, *tmp;

  for (h = 0; h < NHASH; h++) { 
    for (sp = statetab[h]; sp != NULL;) {
      tmp = sp->next;
      free(sp->s);
      free(sp);
      sp = tmp;
    }
    // set table entry to NULL in case of multiple run of the program
    if (statetab[h] != NULL) 
      statetab[h] = NULL;
  }
}

State* AddToken(char *s, Node *node) { 
  int h;
  State *sp;
  h = hash(s);

  /* Have looked up the hash table and it is not present */
  State *head = statetab[h];
  sp = (State *) malloc (sizeof(State));
  if (sp == NULL) perror("State allocation Error");
  sp->s = (char *) malloc(sizeof(char) * (strlen(s) + 1));
  strcpy(sp->s, s);
  //sp->id = id++;
  sp->ptr = node;
  if (head == NULL) {
    head = sp;
    sp->next = NULL;
  } else {
    sp->next = head;
    head = sp;
  }
  statetab[h] = head;
  myprintf("create '%s' at hash table entry %d\n", sp->s, h);

  return sp;
}

//========================================================================
// Generate a C function for the expression
//
// name is a global 
//========================================================================
void GenerateExprFunc(Node *m) {
  Node *root = m;

  fprintf(cfp, "float %s(FILE *fp, int32_or_float* ports, int maxPortPair) {\n", name);
  fprintf(cfp, "  int32_or_float res;\n");
  fprintf(cfp, "  res.f = ");

  do {
    // Get a variable token get whatever is read 
    // from the expression if it is not a variable token
    get_variable();

    // always search from root
    m = root; 

    // Just print what is consumed by get_variable() if leaf is not found 
    if (!SearchLeaf(m)) 
      fprintf(cfp, "%s", token);

  } while (*token != '\0');

  fprintf(cfp, ";\n"); // end of expression conversion
  fprintf(cfp, "  printf(\"Single %s res=%%.11f\\n\", res.f);\n", name);
  fprintf(cfp, "  fprintf(fp, \"%%08x\\n\", res.i);\n");
  fprintf(cfp, "  return res.i;\n");
  fprintf(cfp, "}\n");
}

//========================================================================
// Find the variable leaf node in the expression 
// and print the node's value in the form of "ports[i].f"
//========================================================================
int SearchLeaf(Node *m) {
  if (m->child != NULL) {
    if (SearchLeaf(m->child[0])) return 1;
    if (SearchLeaf(m->child[1])) return 1;
  }

  // print a variable leaf node (i.e. port node)
  if (m->type == VARIABLE && !strcmp(m->value.s, token)) {
    fprintf(cfp, "ports[%d].f", -(m->id) - 1);
    return 1;
  }
  return 0;
}


//========================================================================
// Free tree nodes in bottom-top sequence
//
// Note a child pointer is set to NULL if it points to a node to be freed
// so that we wouldn't free an already freed node
//========================================================================
void CutTree(Node *root, Node *m) {

  if (m != NULL) {
    if (m->child != NULL) {
      // free nonleaf nodes
      CutTree(root, m->child[0]);
      CutTree(root, m->child[1]);
      SearchNode(root, m);
      //assert(m->id > 0);
      free(m->child);
      free(m);
    }
    else {
      // free leaf nodes (VAR or NUM)
      //assert(m->id <= 0);
      SearchNode(root, m);
      if (m->type == VARIABLE) {
        if (m->value.s != NULL) {
          free(m->value.s);
          m->value.s = NULL;
        }
      }
      free(m);
    }
  }
}

//=======================================================================================
// If a node has been visited (m->v = 1), we don't draw it again
//=======================================================================================
void DrawDFG(Node *root, Node *m) {

  if (m != NULL) {
    if (m->child != NULL) {
      m->v = 0;
      DrawDFG(root, m->child[0]);
      DrawDFG(root, m->child[1]);
    }
    if (m->v == 0) {
      draw(root, m);
      m->v = 1;
    }
  }
}

//=========================================================
// Draw node with Dot language
//=========================================================
void draw(Node *node, Node *m) {
  char label[20];
  assert(node != NULL);
  assert(m    != NULL);

  if (node->child != NULL) {
    if (node->child[0] == m || node->child[1] == m) {
      // node label
      fprintf(gfp, "  ");

      // src lable
      if (m->type == VARIABLE)
        sprintf(label, "%s", m->value.s);
      else if (m->type == NUMBER)
        sprintf(label, "%f", m->value.num);
      else
        sprintf(label, "%c(%d)", m->op, m->id);

      // src node
      fprintf(gfp, "\"%s\"", label);

      fprintf(gfp, " -> "); // directed edge 

      // dest label
      sprintf(label, "%c(%d)", node->op, node->id);

      // dest node
      fprintf(gfp, "\"%s\"", label); 

      // the end
      fprintf(gfp, ";\n"); 
    }
    draw(node->child[0], m);
    draw(node->child[1], m);
  }
}

void ResetVisitedNode(Node *m) {
  if (m != NULL) {
    if (m->child != NULL) {
      ResetVisitedNode(m->child[0]);
      ResetVisitedNode(m->child[1]);
    } 
    m->v = 0;
  }
}


void AssignLeafRank(Node *m) {
  if (m != NULL) {
    if (m->child != NULL) {
      m->v = 0;
      AssignLeafRank(m->child[0]);
      AssignLeafRank(m->child[1]);
    } 

    // mark leaf node 
    if (m->v == 0) {
      if (m->type == VARIABLE)
        fprintf(gfp, "\"%s\"; ", m->value.s);
      else if (m->type == NUMBER)
        fprintf(gfp, "\"%f\"; ", m->value.num);

      m->v  = 1; 
    }
  }
}

void GenerateDotFile(Node *root) {
  fprintf(gfp, "digraph G {\n");
  fprintf(gfp, "  size=\"6,6\";\n");
  fprintf(gfp, "  node [shape=circle style=filled width=1 height=1 fillcolor=lightblue fontsize=17];\n");

  fprintf(gfp, "  { rank = same; ");
  AssignLeafRank(root);
  fprintf(gfp, "}\n\n");

  ResetVisitedNode(root);

  DrawDFG(root, root);

  fprintf(gfp, "}\n");
}


//=========================================================
// Null a child pointer if it points to a node to be freed
//=========================================================
void SearchNode(Node *node, Node *m) {
  if (node->child != NULL) {
    if (node->child[0] == m) {
      //printf("set node %d left child to NULL\n", node->id);
      node->child[0] = NULL;
    }
    if (node->child[1] == m) {
      //printf("set node %d right child to NULL\n", node->id);
      node->child[1] = NULL;
    }
    if (node->child[0] != NULL) SearchNode(node->child[0], m);
    if (node->child[1] != NULL) SearchNode(node->child[1], m);
  }
}

//=========================================================
// Generate sources and destination of a nonleaf node
//=========================================================
void GenerateNonLeafNodes(Node *m, Node *root, int nid) {

  int sid;
  int isConst = 0;
  char *nop;

  if (m->child != NULL) {
    if (m->id == nid) {
      nop = MmOp(m->op);
      fprintf(fp, "DFG[%d]->op = %s;\n", m->id, nop); free(nop);
      fprintf(fp, "DFG[%d]->opSrc = (int *) malloc(sizeof(int) * 2);\n", m->id);

      if (root->id == nid) // sink node has only 1 destination
        fprintf(fp, "DFG[%d]->opDest = (int *) malloc(sizeof(int) * 1);\n", m->id);
      else
        fprintf(fp, "DFG[%d]->opDest = (int *) malloc(sizeof(int) * %d);\n",
          m->id, visit(root, m->id, 0, 0));

      // op source A (port or non-port)
      if (m->child[0]->type == NUMBER || m->child[1]->type == NUMBER) {
        // A constant input exists
        if (m->child[0]->type == NUMBER) {
          assert (m->child[1]->type != NUMBER);
          sid = m->child[1]->id;
        } else {
          assert (m->child[0]->type != NUMBER);
          sid = m->child[0]->id;
        }
      } else {
          sid = m->child[0]->id;
      }

      sid = (sid < 0) ? -sid-1 : sid;
      fprintf(fp, "DFG[%d]->opSrc[0] = %d;\n", m->id, sid);
      
      // op source B (const)
      if (m->child[0]->type == NUMBER || m->child[1]->type == NUMBER) {
        fprintf(fp, "DFG[%d]->opSrc[1] = %d;\n", m->id, sid);
        isConst = 1;
      }
      // op source B (non-const)
      else {
        sid = m->child[1]->id;
        sid = (sid < 0) ? -sid-1 : sid;
        fprintf(fp, "DFG[%d]->opSrc[1] = %d;\n", m->id, sid);
      }

      // op destination 
      if (root->id == nid) { // sink node's destination is SINK
         fprintf(fp, "DFG[%d]->opDest[0] = SINK;\n", nid);
         fprintf(fp, "DFG[%d]->opDestNu = 1;\n", nid);
      }
      else 
        fprintf(fp, "DFG[%d]->opDestNu = %d;\n", nid, visit(root, m->id, 0, 1));

      // DFG node's other attributes
      fprintf(fp, "DFG[%d]->RegFileTable = NULL;\n", nid);
      fprintf(fp, "DFG[%d]->opConst = %d;\n", nid, isConst);
      fprintf(fp, "\n");
    }

    GenerateNonLeafNodes(m->child[0], root, nid);
    GenerateNonLeafNodes(m->child[1], root, nid);
  }
}

//========================================================================
// Mark each nonleaf node with its node ID values (p, p + 1, p + m - 1)
// and return the total number of leaf and nonleaf nodes (m + n)
//========================================================================
int MarkNode(Node *m, int p) {
  // for each child of node m, from left to right
  if (m->child != NULL) {
    p = MarkNode(m->child[0], p);
    p = MarkNode(m->child[1], p);

    // evaluate node m
    m->id = p++;
    myprintf("Node %d op = %c\n", m->id, m->op);
  }
  return p;
}

//========================================================================
// Mark each leaf node with its port ID values (-1, -2, .. -n) 
// and return the total number of input ports (P1, P2, P3 ... Pn)
//========================================================================
void MarkLeaf(Node *m, int *n, int *p) {
  if (m->child != NULL) {
    MarkLeaf(m->child[0], n, p);
    MarkLeaf(m->child[1], n, p);
  }

  // mark leaf node (i.e. port node)
  // Set node as visited after it has been visited.
  // It is possible that children of non-leaf nodes 
  // point to the same leaf node 
  if (m->type == VARIABLE) {
    if (m->v == 0) {
      m->v  = 1; 
      m->id = *p;
      assert(m->id < 0);
      myprintf("%s (port %d)\n", m->value.s, -(*p));
      (*p)--;
      (*n)++;
    }
  }
}

//========================================================================
// Generate source and destination of each input port (nonleaf node)
//========================================================================
void GenerateLeafNodes(Node *m, Node *root) {
  // traverse from leaves to root
  int pid, nid; 
  char *nop;
  
  // Skip nonleaf nodes and go to leaves nodes
  if (m->child != NULL) {
    GenerateLeafNodes(m->child[0], root);
    GenerateLeafNodes(m->child[1], root);
  }
 
  // Here comes a leaf node
  if (m->type == VARIABLE) {
    if (m->v) {
      m->v = 0;
      pid = -(m->id) - 1;
      assert(pid >= 0);
      nid = m->id;
      nop = MmOp(m->op);
      fprintf(fp, "DFG[%d]->op = %s;\n", pid, nop); free(nop);
      fprintf(fp, "DFG[%d]->opSrc  = (int *) malloc(sizeof(int) * 1);\n", pid);
      fprintf(fp, "DFG[%d]->opDest = (int *) malloc(sizeof(int) * %d);\n",
          pid, visit(root, m->id, 0, 0));

      fprintf(fp, "DFG[%d]->opSrc[0] = P%d;\n", pid, pid+1);
      // Search non-leaf nodes
      fprintf(fp, "DFG[%d]->opDestNu = %d;\n", pid, visit(root, nid, 0, 1));
      fprintf(fp, "DFG[%d]->RegFileTable = NULL;\n", pid);
      fprintf(fp, "DFG[%d]->opConst = %d;\n", pid, 0);
      fprintf(fp, "\n");
    }
  }
}

//========================================================================
// 1. Find all the destinations of a node:
//    If id of a node's child is equal to nid, then the node's id 
//    is a destination of the child. The destination means the 
//
//   (src id)      (src id)
//   left child   right child 
//       \           /
//             + 
//         parent node 
//         (dest id)
//
// 2. Collect the number of destinations of a node and return it
// 3. Print stdout if dump = 1
//========================================================================
int visit(Node *m, int nid, int destNu, int dump) {

  // for each child of node m, from left to right
  if (m->child != NULL) {
    if (m->child[0]->id == nid || m->child[1]->id == nid) {
      if (dump) {
        fprintf(fp, "DFG[%d]->opDest[%d] = %d;\n", 
            nid < 0 ? -nid - 1 : nid, destNu, m->id);
      }
      destNu++;
    }

    destNu = visit(m->child[0], nid, destNu, dump);
    destNu = visit(m->child[1], nid, destNu, dump);
  }
  return destNu;
}


//========================================================================
// Can we print a tree like a tree
//========================================================================
void PrintTree(Node *m) {
  Node *node = m;
  myprintf("id=%d op=%c type=%d ptr=%p\n", node->id, node->op, node->type, node);
  if (node->child != NULL) {
    PrintTree(node->child[0]);
    PrintTree(node->child[1]);
  }
}

