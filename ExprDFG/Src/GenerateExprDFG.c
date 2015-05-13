/* 
 * Parse a simple expression (variable, number and delimiter)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <assert.h>
#include "State.h"
#include "ExprParser.h"

#define OTHERS    0
#define VARIABLE  1
#define NUMBER    2
#define DELIMITER 3

/* hash: compute hash value for an array of NPREF strings */
unsigned int hash (char *s) {
  unsigned int h;
  unsigned char *p;
  int i;
  const int MULTIPLIER = 37;
  
  h = 0;
  for (p = (unsigned char *) s; *p != '\0'; p++)
    h = MULTIPLIER * h + *p;
  return h % NHASH;
}

/* lookup: search for id; create if requested. */
/* returns pointer if present or created; NULL if not. */
/* creation doesn't strdup so strings mustn't change later. */
State* lookup(char *s, int create) {
  int i, h;
  State *sp;
  static int id;
  h = hash(s);
  //printf("index(%s) = %d\n", s, h);

  for (sp = statetab[h]; sp != NULL; sp = sp -> next) {
    if (strcmp(s, sp->s) == 0)
      return sp;
  }

  /* Have looked up the hash table and it is not present */
  if (create) {
    State *head = statetab[h];
    sp = (State *) malloc (sizeof(State));
    if (sp == NULL) perror("State allocation Error");
    sp->s = (char *) malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(sp->s, s);
    sp->id = id++;
    if (head == NULL) {
      head = sp;
      sp->next = NULL;
    } else {
      sp->next = head;
      head = sp;
    }
    statetab[h] = head;
    printf("create '%s' at table entry %d\n", sp->s, h);
  }
  return sp;
}

void displayState(void) {
  int h;
  State *sp;
  for (h = 0; h < NHASH; h++)
    for (sp = statetab[h]; sp != NULL; sp = sp -> next)
      printf("<%s, %d>\n", sp->s, sp->id);
}
    
void freeState(void) {
  int h;
  State *sp;
  for (h = 0; h < NHASH; h++) {
    for (sp = statetab[h]; sp != NULL; sp = sp -> next) {
      free(sp->s);
      free(sp);
    }
  }
}
    


char token[80];
char token_type;

/* Parser entry point */
void eval_exp(double *answer)
{
  // Scan the entire expression and 
  // calculate the total number of input ports
  char *s = (char *) malloc (sizeof(char) * (strlen(expr) + 1));

  strcpy(s, expr);

  do {
    get_token();
    if (token_type == VARIABLE) {
      PORT_NU++;
      lookup(token, 1);
    }
  } while (*token != 0);

  displayState();

  expr = s;

  get_token();
  if(!*token) {
    serror(2);
    return;
  }
  eval_exp2(answer);

  free(s);
}

/* Add/subtract two terms */
void eval_exp2(double *answer)
{
  char op;
  double temp;
  int op_id;
  int source_id;
  int dest_nu;

  //source_id = eval_exp3(answer);
  source_id = eval_exp6(answer);
  dest_nu = 0;
  int first = 1;

  while ((op = *token) == '+' || op == '-') {

    //if (!first) source_id = op_id;
    //op_id = PORT_NU++;

    //printf("DFG[%d]->opDest[%2d] = %d;\n", source_id, dest_nu, op_id);
    //printf("DFG[%d]->opSrc[%2d] = %d;\n",  op_id, 0, source_id);
    get_token();

    //eval_exp3(&temp);
    source_id = eval_exp6(&temp);

    //printf("DFG[%d]->opSrc[%2d] = %d;\n",  op_id, 1, source_id);
    switch(op) {
      case '-':
        *answer = *answer - temp; 
        break;
      case '+':
        *answer = *answer + temp; 
        break;
    }
    //dest_nu++;

    if (first == 1) {
      first = 0;
    }
  }
}



/* Process a parenthesized expression */
int eval_exp6(double *answer)
{
  if (*token == '(') {
    get_token();
    eval_exp2(answer);
    if (*token != ')')
      serror(1);
    get_token();
  }
  //else atom(answer);
  else 
    return (eval_exp7(answer));
}

/* Get the actual value of a number */
//void atom(double *answer)
int eval_exp7(double *answer)
{
  State *sp;

  if (token_type == NUMBER) {
    *answer = atof(token);
    get_token();
    return NUMBER;
  }
  else if (token_type == VARIABLE) {
    //printf("looking up %s\n", token);
    sp = lookup(token, 0);
    assert(sp != NULL);
    //printf("DFG[%d]->opSrc[0] = P%d;\n", sp->id, sp->id);
    
    get_token();
    return sp->id;
  }

  Leaf *n = (Leaf *) malloc (sizeof(Leaf));
  n->type = token_type;

  if (n->type == VARIABLE) {
    n->value.s = (char *) malloc (sizeof(char) * (strlen(token) + 1));
    strcpy(n->value.s, token); 
    return n;
  } 
  else if (n->type == NUMBER) {
    n->value.num =  atof(token);
    return n;
  }

  serror(0);
  return -1;
}

/* Display a syntax error */
void serror(int error) 
{
  static char *e[] = {
    "syntax error",
    "unbalanced parentheses",
    "no expression present"
  };
  printf("%s\n", e[error]);
}

/* Return true if c is a delimter */
int isdelim(char c)
{
  if (strchr(" +-/*%^=()", c) || c == 9 || c == 'r' || c == 0)
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
  if (!*expr) return ;

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

  //printf("The next token is %s\n", token);
}

/*
// Multiply/divide two terms 
void eval_exp3(double *answer)
{
  char op;
  double temp;
  int id;
  eval_exp4(answer);
  while ((op = *token) == '*' || op == '/' || op == '%') {
    id = PORT_NU++;
    get_token();
    eval_exp4(&temp);
    switch(op) {
      case '*':
        *answer = *answer * temp; 
        break;
      case '/':
        *answer = *answer / temp; 
        break;
      case '%':
        *answer = (int) *answer % (int) temp; 
        break;
    }
  }
}

// Process an exponent 
void eval_exp4(double *answer)
{
  double temp, ex;
  int t;
  eval_exp5(answer);
  if (*token == '^') {
    get_token();
    eval_exp4(&temp);
    ex = *answer;
    if (temp == 0.0) {
      *answer = 1.0;
      return;
    }
    for ( t = temp - 1; t > 0; --t)
      *answer = (*answer) * ex; 
  }
}

// Evaluate a unary + or -
void eval_exp5(double *answer)
{
  char op = 0;
  int id;
  if ((token_type == DELIMITER) && (*token == '+' || *token == '-')) {
    op = *token;
    get_token();
  }
  eval_exp6(answer);
  if (op == '-') 
    *answer = - (*answer);
}
*/
