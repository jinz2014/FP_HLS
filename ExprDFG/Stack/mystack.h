#ifndef STACK_
#define STACK_

/* mystack.h -- Stack declaration and function prototypes:  */
/* Set a static stack of depth 200 */

#define DEPTH 200
//#define SAMPLE

#ifdef SAMPLE
typedef struct
{
    float v[DEPTH];
    int top;
} Stack;

void push(Stack *S, float val);
float pop(Stack *S);
void init(Stack *S);
int full(Stack *S);
void MyStackPrint(Stack *S);

#else

#include "ExprParser.h"

typedef struct
{
    Node *v[DEPTH];
    int top;
} Stack;

void push(Stack *S, Node *p);
Node *pop(Stack *S);
Node *peek(Stack *S);
void init(Stack *S);
int full(Stack *S);
int empty(Stack *S);
void MyStackPrint(Stack *S);

#endif

#endif /* STACK_ */
