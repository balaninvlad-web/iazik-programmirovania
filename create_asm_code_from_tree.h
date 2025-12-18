#ifndef CREATE_ASM_CODE_FROM_TREE_H
#define CREATE_ASM_CODE_FROM_TREE_H

#include "tree_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char* name;
    int start_label;
    int local_var_count;
} FunctionInfo;

typedef struct
{
    char* name;
    int address;
    int is_local;
} VariableInfo;

typedef struct
{
    FILE* output;
    int label_counter;
    int var_counter;
    int temp_counter;
    VariableInfo* var_table;
    FunctionInfo* func_table;
    int var_capacity;
    int func_capacity;
    int var_count;
    int func_count;
    char* current_func;
    int in_function;
} CodeGenContext;

CodeGenContext* CtorCodeGen (FILE* output);
void DtorCodeGen (CodeGenContext* ctx);

void GenerateCode (CodeGenContext* ctx, Node* node);

int NewLabel (CodeGenContext* ctx);
int GetVarAddress (CodeGenContext* ctx, const char* var_name);
int AddVariable (CodeGenContext* ctx, const char* var_name, int is_local);
int AddFunction (CodeGenContext* ctx, const char* func_name);
void EnterFunction (CodeGenContext* ctx, const char* func_name);
void ExitFunction (CodeGenContext* ctx);

void GenExpression (CodeGenContext* ctx, Node* node);
void GenAssignment (CodeGenContext* ctx, Node* node);
void GenSequence (CodeGenContext* ctx, Node* node);
void GenIf (CodeGenContext* ctx, Node* node);
void GenWhile (CodeGenContext* ctx, Node* node);
void GenVarDecl (CodeGenContext* ctx, Node* node);
void GenFuncDecl (CodeGenContext* ctx, Node* node);
void GenReturn (CodeGenContext* ctx, Node* node);
void GenFuncCall (CodeGenContext* ctx, Node* node);

#endif
