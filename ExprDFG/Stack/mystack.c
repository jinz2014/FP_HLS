#include<stdio.h>
#include "mystack.h"

#ifdef SAMPLE
void push(Stack *S, float val)
{
    S->v[ S->top ] = val; 
   (S->top)++;    
/*  Equivalent to: S->v[ (S->top)++ ] = val;   */
}

float pop(Stack *S)
{
    (S->top)--;
    return (S->v[S->top]);
/*  Equivalent to: return (S->v[--(S->top)]);  */
}

void init(Stack *S)
{
    S->top = 0;
}

int full(Stack *S)
{
    return (S->top >= DEPTH);
}
void MyStackPrint(Stack *S)
{
    int i;
    if (S->top == 0)
       printf("Stack is empty.\n");
    else
    {
       printf("Stack contents: ");
       for (i=0;i<S->top;i++)
       {
          printf("%g  ",S->v[i]); 
       }
       printf("\n");
    }
}

#else

#include "ExprParser.h"

void push(Stack *S, Node *p)
{
    S->v[ S->top ] = p; 
   (S->top)++;    
/*  Equivalent to: S->v[ (S->top)++ ] = val;   */
}

Node *peek(Stack *S)
{
    return (S->v[S->top - 1]);
}


Node *pop(Stack *S)
{
    (S->top)--;
    return (S->v[S->top]);
/*  Equivalent to: return (S->v[--(S->top)]);  */
}

void init(Stack *S)
{
    S->top = 0;
}

int full(Stack *S)
{
    return (S->top >= DEPTH);
}

int empty(Stack *S) 
{
    return (S->top == 0);
}

void MyStackPrint(Stack *S)
{
    int i;
    if (S->top == 0)
       printf("Stack is empty.\n");
    else
    {
      // Since the contents are pointers, use field data instead
       printf("Stack contents: ");
       for (i=0;i<S->top;i++)
       {
          printf("%d %c %p %p\n", S->v[i]->id, S->v[i]->op, 
            S->v[i]->child[0], S->v[i]->child[1]);
       }
       printf("\n");
    }
}

#endif
