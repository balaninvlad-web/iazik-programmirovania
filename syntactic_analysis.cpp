#include "lexical_analysis.h"
#include "tree_base.h"
#include "syntactic_analysis.h"

Getter* CtorGetter (Lexer* lexer)
{
    Getter* getter = (Getter*) calloc (1, sizeof(Getter));
    if (!getter) return NULL;

    getter->lexer = lexer;
    getter->current_token = 0;
    getter->error_count = 0;

    return getter;
}

void DtorGetter (Getter* getter)
{
    if (getter)
        free (getter);
}

Token* CurrentToken (Getter* getter)
{
    if (!getter || !getter->lexer || getter->current_token >= getter->lexer->count)
        return NULL;

    return &getter->lexer->tokens[getter->current_token];
}

void Advance (Getter* getter)
{
    if (getter && getter->current_token < getter->lexer->count)
        getter->current_token++;
}

bool Match (Getter* getter, MyTokenType type)
{
    Token* token = CurrentToken (getter);
    return token && token->type == type;
}

bool Expect (Getter* getter, MyTokenType type, const char* error_msg)
{
    Token* token = CurrentToken (getter);

    if (!token || token->type != type)
    {
        if (token)
        {
            fprintf (stderr, "ќшибка парсера: %s. ќжидалс€ %s, получен %s\n",
                    error_msg,
                    TokenTypeToString (type),
                    TokenTypeToString (token->type));
        }
        else
        {
            fprintf (stderr, "ќшибка парсера: %s. ќжидалс€ %s, но токены закончились\n",
                    error_msg, TokenTypeToString (type));
        }
        getter->error_count++;
        return false;
    }

    Advance (getter);
    return true;
}

Node* GetPrimary (Getter* getter)
{
    Token* token = CurrentToken (getter);
    if (!token) return NULL;

    if (token->type == TOK_NUMBER)
    {
        Node* node = CreateNumber (token->value.number);
        Advance (getter);
        return node;
    }

    if (token->type == TOK_IDENTIFIER)
    {
        Token* next = NULL;
        if (getter->current_token + 1 < getter->lexer->count)
            next = &getter->lexer->tokens[getter->current_token + 1];

        if (next && next->type == TOK_LPAREN)
        {
            char* func_name = strdup (token->value.identifier);
            Advance(getter); // съедаем им€ функции

            Advance(getter); // съедаем '('
            Node* args = GetExpression (getter);
            Expect (getter, TOK_RPAREN, "ќжидалась ')' после аргументов функции");

            Node* node = CreateFunctionCall (func_name, args);
            free (func_name);
            return node;
        }
        else
        {
            Node* node = CreateVariable (token->value.identifier);
            Advance (getter);
            return node;
        }
    }

    if (token->type == TOK_LPAREN)
    {
        Advance (getter); // съедаем '('
        Node* expr = GetExpression (getter);
        Expect (getter, TOK_RPAREN, "ќжидалась ')' после выражени€");
        return expr;
    }

    fprintf (stderr, "ќшибка: ожидалось число, переменна€ или '('\n");
    getter->error_count++;
    return NULL;
}

Node* GetUnary (Getter* getter)  // унарные +- на них похер
{
    Token* token = CurrentToken (getter);

    if (token->type == TOK_PLUS)
    {
        Advance (getter); // плюс ничего не мен€ет
        return GetUnary (getter);
    }

    if (token->type == TOK_MINUS)
    {
        Advance (getter); // съедаем '-'
        Node* operand = GetUnary (getter);
        return CreateOperation (NODE_SUB, CreateNumber(0), operand);
    }

    return GetPrimary (getter);
}

Node* GetFactor (Getter* getter) // */
{
    Node* node = GetUnary (getter);

    while (1)
    {
        Token* token = CurrentToken (getter);
        if (!token) break;

        NodeType op_type = NODE_EMPTY;
        if (token->type == TOK_MULTIPLY)
            op_type = NODE_MUL;
        else if (token->type == TOK_DIVIDE)
            op_type = NODE_DIV;
        else
            break;

        Advance (getter);
        Node* right = GetUnary (getter);
        node = CreateOperation (op_type, node, right);
    }

    return node;
}


Node* GetTerm (Getter* getter) // +-
{
    Node* node = GetFactor(getter);

    while (1)
    {
        Token* token = CurrentToken (getter);
        if (!token) break;

        NodeType op_type = NODE_EMPTY;
        if (token->type == TOK_PLUS)
            op_type = NODE_ADD;
        else if (token->type == TOK_MINUS)
            op_type = NODE_SUB;
        else
            break;

        Advance (getter);
        Node* right = GetFactor (getter);
        node = CreateOperation (op_type, node, right);
    }

    return node;
}

Node* GetComparison (Getter* getter)//  —равнени€ (==, !=, >, <)
{
    Node* node = GetTerm (getter);

    while (1)
    {
        Token* token = CurrentToken (getter);
        if (!token) break;

        NodeType op_type = NODE_EMPTY;
        if (token->type == TOK_EQ)
            op_type = NODE_EQ;
        else if (token->type == TOK_NE)
            op_type = NODE_NE;
        else if (token->type == TOK_GT)
            op_type = NODE_GT;
        else if (token->type == TOK_LT)
            op_type = NODE_LT;
        else
            break;

        Advance (getter);
        Node* right = GetTerm (getter);
        node = CreateOperation (op_type, node, right);
    }

    return node;
}

Node* GetExpression(Getter* getter)
{
    Node* node = GetComparison(getter);
    if (!node) {
        return NULL;
    }
    return node;
}

Node* GetAssignment (Getter* getter)
{
    Token* token = CurrentToken (getter);
    char* var_name = strdup (token->value.identifier);
    Node* variable = CreateVariable (var_name);
    free (var_name);

    Advance (getter); // съедаем переменную

    Expect (getter, TOK_ASSIGN, "ќжидалось '=' в присваивании");

    Node* value = GetExpression (getter);
    Expect (getter, TOK_SEMICOLON, "ќжидалось ';' после присваивани€");

    return CreateAssignment (variable, value);
}

Node* GetVarDecl (Getter* getter)
{
    Token* type_token = CurrentToken (getter);
    NodeType var_type;

    switch (type_token->type)
    {
        case TOK_TYPE_INT: var_type = NODE_TYPE_INT; break;
        case TOK_TYPE_CHAR: var_type = NODE_TYPE_CHAR; break;
        case TOK_TYPE_DOUBLE: var_type = NODE_TYPE_DOUBLE; break;
        default:
            fprintf (stderr, "ќшибка: ожидалс€ тип переменной\n");
            getter->error_count++;
            return NULL;
    }

    Advance(getter); // съедаем тип

    Token* id_token = CurrentToken (getter);
    if (!Expect (getter, TOK_IDENTIFIER, "ќжидалось им€ переменной"))
        return NULL;

    char* var_name = strdup (id_token->value.identifier);

    Node* init_value = NULL;
    if (Match(getter, TOK_ASSIGN))
    {
        Advance (getter); // съедаем '='
        init_value = GetExpression (getter);
        if (!init_value)
        {
            free (var_name);
            return NULL;
        }
    }

    if (!Expect (getter, TOK_SEMICOLON, "ќжидалось ';' после объ€влени€ переменной")) {
        free(var_name);
        if (init_value) FreeTree (init_value);
        return NULL;
    }

    Node* var_decl = CreateVarDeclaration (var_type, var_name, init_value);
    free (var_name);
    return var_decl;
}

Node* GetReturn (Getter* getter)
{
    Expect(getter, TOK_RETURN, "ќжидалось 'return'");

    Node* expr = GetExpression (getter);
    Expect (getter, TOK_SEMICOLON, "ќжидалось ';' после return");

    return CreateReturn (expr);
}

Node* GetWhile (Getter* getter)
{
    Expect (getter, TOK_WHILE, "ќжидалось 'while'");
    Expect (getter, TOK_LPAREN, "ќжидалось '(' после while");

    Node* condition = GetExpression (getter);

    Expect (getter, TOK_RPAREN, "ќжидалось ')' после услови€ while");

    Node* body = GetStatement (getter);

    return CreateOperation (NODE_WHILE, condition, body);
}

Node* GetIf (Getter* getter)
{
    Expect (getter, TOK_IF, "ќжидалось 'if'");
    Expect (getter, TOK_LPAREN, "ќжидалось '(' после if");

    Node* condition = GetExpression (getter);

    Expect (getter, TOK_RPAREN, "ќжидалось ')' после услови€");

    Node* body = GetStatement (getter);

    return CreateOperation (NODE_IF, condition, body);
}


Node* GetBlock (Getter* getter) // Ѕлок { }
{
    Expect (getter, TOK_LBRACE, "ќжидалось '{'");

    Node* body = GetStatements (getter);

    Expect (getter, TOK_RBRACE, "ќжидалось '}'");

    if (!body || body->type == NODE_EMPTY) // пустой
    {
        if (body) FreeTree (body);
        return CreateEmpty ();
    }

    return body;
}

Node* GetStatement (Getter* getter) // один оператор
{
    Token* token = CurrentToken (getter);
    if (!token) return CreateEmpty();

    #ifdef DEBUG
        printf("DEBUG GetStatement: токен %d: %s",
               getter->current_token,
               TokenTypeToString (token ? token->type : TOK_EOF));
        if (token && token->type == TOK_IDENTIFIER)
            printf (" ('%s')", token->value.identifier);
        else if (token && token->type == TOK_NUMBER)
            printf (" (%g)", token->value.number);
        printf("\n");
    #endif

    switch (token->type)
    {
        case TOK_TYPE_INT:
        case TOK_TYPE_CHAR:
        case TOK_TYPE_DOUBLE:
            return GetVarDecl (getter);

        case TOK_IF:
            return GetIf (getter);
        case TOK_WHILE:
            return GetWhile (getter);
        case TOK_RETURN:
            return GetReturn (getter);

        case TOK_LBRACE:
            return GetBlock (getter);

        case TOK_IDENTIFIER:
        {
            Token* next = NULL;
            if (getter->current_token + 1 < getter->lexer->count)
                next = &getter->lexer->tokens[getter->current_token + 1];

            if (next && next->type == TOK_ASSIGN)
                return GetAssignment(getter);
            else
            {
                Expect (getter, TOK_ASSIGN, "ќжидалось присваивание");
                getter->error_count++;
                return NULL;
            }
        }

        case TOK_RBRACE:
            return CreateEmpty ();

        default:
            fprintf (stderr, "ќшибка: неожиданный токен в операторе: %s\n",
                    TokenTypeToString(token->type));
            getter->error_count++;
            return NULL;
    }
}

Node* GetStatements (Getter* getter)  // именно последовательность операторов
{
    Node* first = NULL;

    while (1)
    {
        Token* token = CurrentToken (getter);
        if (!token || token->type == TOK_RBRACE)
            break;

        Node* stmt = GetStatement (getter);
        if (!stmt)
        {
            if (first) FreeTree (first);
            return NULL;
        }

        if (stmt->type == NODE_EMPTY)
        {
            FreeTree(stmt);
            continue;
        }

        if (!first)
            first = stmt;
        else
            first = CreateSequence(first, stmt);
    }

    if (!first)
        return CreateEmpty();

    return first;
}

Node* GetFunction (Getter* getter)
{
    if (!Expect(getter, TOK_DECLARE, "ќжидалось объ€вление функции"))
        return NULL;

    Token* type_token = CurrentToken (getter);
    NodeType return_type = {};

    switch (type_token->type)
    {
        case TOK_TYPE_INT: return_type = NODE_TYPE_INT; break;
        case TOK_TYPE_CHAR: return_type = NODE_TYPE_CHAR; break;
        case TOK_TYPE_DOUBLE: return_type = NODE_TYPE_DOUBLE; break;
        default:
            fprintf (stderr, "ќшибка: ожидалс€ тип возврата функции\n");
            getter->error_count++;
            return NULL;
    }

    Advance(getter);

    Token* id_token = CurrentToken(getter);
    if (!Expect(getter, TOK_IDENTIFIER, "ќжидалось им€ функции"))
        return NULL;

    char* func_name = strdup(id_token->value.identifier);

    if (!Expect(getter, TOK_LPAREN, "ќжидалось '(' после имени функции"))
    {
        free(func_name);
        return NULL;
    }

    Node* params = NULL;  // TODO: параметры

    if (!Expect(getter, TOK_RPAREN, "ќжидалось ')' после параметров"))
    {
        free(func_name);
        return NULL;
    }

    Node* body = GetBlock(getter);
    if (!body)
    {
        free(func_name);
        return NULL;
    }

    Node* func_decl = CreateFunctionDeclaration(return_type, func_name, NULL, body);
    free (func_name);

    return func_decl;
}

Node* GetProgram (Getter* getter)
{
    Node* func = GetFunction (getter);
    // TODO сделать чтобы было несколько функций
    if (getter->error_count == 0)
    {
        Token* token = CurrentToken (getter);
        if (token && token->type != TOK_EOF)
        {
            fprintf (stderr, "ќшибка: лишние токены после функции\n");
            getter->error_count++;
        }
    }

    if (getter->error_count > 0)
    {
        if (func) FreeTree (func);
        return NULL;
    }

    return func;
}

