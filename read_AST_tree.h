#ifndef READ_AST_TREE_H
#define READ_AST_TREE_H
#include "tree_base.h"

#ifdef DEBUG
#define PARSER_DEBUG(...) printf("[PARSER] " __VA_ARGS__)
#else
#define PARSER_DEBUG(...)
#endif

struct ParserState
{
    const char* input;
    int pos;
    int line;
    int col;
};

void CtorParser (ParserState* state, const char* str);
Node* CreateNodeFromToken (const char* token);
Node* ParseExpression (ParserState* str);
Node* ParseLispAST (const char* str);

#endif

