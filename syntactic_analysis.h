#ifndef SYNTACTIC_ANALYSIS_H
#define SYNTACTIC_ANALYSIS_H

struct Getter
{
    Lexer* lexer;
    int current_token;
    int error_count;
};

Getter* CtorGetter (Lexer* lexer);
void DtorGetter (Getter* getter);
Token* CurrentToken (Getter* getter);
void Advance (Getter* getter);
bool Match (Getter* getter, MyTokenType type);
bool Expect (Getter* getter, MyTokenType type, const char* error_msg);

Node* GetProgram (Getter* getter);
Node* GetFunction (Getter* getter);
Node* GetBlock (Getter* getter);
Node* GetStatements (Getter* getter);
Node* GetStatement (Getter* getter);
Node* GetVarDecl (Getter* getter);
Node* GetAssignment (Getter* getter);
Node* GetWhile (Getter* getter);
Node* GetIf (Getter* getter);
Node* GetReturn (Getter* getter);
Node* GetExpression (Getter* getter);
Node* GetComparison (Getter* getter);
Node* GetTerm (Getter* getter);
Node* GetFactor (Getter* getter);
Node* GetUnary (Getter* getter);
Node* GetPrimary (Getter* getter);

#endif
