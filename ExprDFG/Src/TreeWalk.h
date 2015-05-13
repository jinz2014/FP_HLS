#ifndef TREE_WALK_
#define TREE_WALK_

#include "ExprParser.h"
#include "mystack.h"
void preorder_tree_walk(Stack *S, Node *root);
void inorder_tree_walk(Stack *S, Node *root);
void postorder_tree_walk(Stack *S, Node *root);

#endif /* TREE_WALK_ */
