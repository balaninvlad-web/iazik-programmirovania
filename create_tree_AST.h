#ifndef CREATE_TREE_AST_H
#define CREATE_TREE_AST_H
#include <stdio.h>
#include <string.h>
#include "tree_base.h"

void DumpAST (Node* node, FILE* file);
void NodeDump (Node* node, FILE* file, int depth, int is_child);
void DumpAST (Node* node, FILE* file);

#endif
