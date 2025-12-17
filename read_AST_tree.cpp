#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "read_AST_tree.h"

void CtorParser (ParserState* state, const char* str)
{
    state->input = str;
    state->pos = 0;
    state->line = 1;
    state->col = 1;
}

static void SkipSpaces (ParserState* str)
{
    while (str->input[str->pos])
    {
        char c = str->input[str->pos];
        if (c == '\n')
        {
            str->line++;
            str->col = 1;
            str->pos++;
        }
        else if (isspace(c))
        {
            str->col++;
            str->pos++;
        }
        else
            break;
    }
}

static int IsNilAtPos (ParserState* str)
{
    return strncmp (str->input + str->pos, "nil", 3) == 0 &&
           (isspace (str->input[str->pos + 3]) || str->input[str->pos + 3] == ')');
}

char* ReadToken (ParserState* str)
{
    SkipSpaces (str);

    if (!str->input[str->pos])
        return NULL;

    if (str->input[str->pos] == '(' || str->input[str->pos] == ')')
    {
        char* token = (char*) calloc (2, sizeof (char));
        token[0] = str->input[str->pos];
        token[1] = '\0';
        str->pos++;
        str->col++;
        return token;
    }

    const char* start = str->input + str->pos;
    int len = 0;

    while (str->input[str->pos + len] &&
           !isspace(str->input[str->pos + len]) &&
           str->input[str->pos + len] != '(' &&
           str->input[str->pos + len] != ')')
        len++;

    if (len == 0)
        return NULL;

    char* token = (char*) calloc(len + 1, sizeof (char));
    strncpy (token, start, len);
    token[len] = '\0';

    str->pos += len;
    str->col += len;

    PARSER_DEBUG("Read token: '%str' at line %d, col %d\n", token, str->line, str->col);
    return token;
}

Node* CreateNodeFromToken (const char* token)
{
    if (!token) return NULL;

    char* endptr = {};
    double num = strtod (token, &endptr);
    if (*endptr == '\0')
    {
        PARSER_DEBUG("Creating NUMBER node: %g\n", num);
        return CreateNumber (num);
    }

    if (strcmp (token, "+") == 0)     return CreateOperation (NODE_ADD, NULL, NULL);
    if (strcmp (token, "-") == 0)     return CreateOperation (NODE_SUB, NULL, NULL);
    if (strcmp (token, "*") == 0)     return CreateOperation (NODE_MUL, NULL, NULL);
    if (strcmp (token, "/") == 0)     return CreateOperation (NODE_DIV, NULL, NULL);
    if (strcmp (token, ">") == 0)     return CreateOperation (NODE_GT, NULL, NULL);
    if (strcmp (token, "<") == 0)     return CreateOperation (NODE_LT, NULL, NULL);
    if (strcmp (token, "=") == 0)     return CreateOperation (NODE_ASSIGNMENT, NULL, NULL);
    if (strcmp (token, ";") == 0)     return CreateOperation (NODE_SEQUENCE, NULL, NULL);
    if (strcmp (token, "if") == 0)    return CreateOperation (NODE_IF, NULL, NULL);
    if (strcmp (token, "while") == 0) return CreateOperation (NODE_WHILE, NULL, NULL);
    if (strcmp (token, "ret") == 0)   return CreateOperation (NODE_RETURN, NULL, NULL);

    PARSER_DEBUG("Creating VARIABLE node: %str\n", token);
    return CreateVariable (token);
}

Node* ParseExpression (ParserState* str)
{
    SkipSpaces (str);

    if (IsNilAtPos(str))
    {
        PARSER_DEBUG("Found nil at line %d, col %d\n", str->line, str->col);
        str->pos += 3;
        str->col += 3;
        return NULL;
    }

    if (str->input[str->pos] != '(')
    {
        char* token = ReadToken (str);
        Node* node = CreateNodeFromToken (token);
        free (token);
        return node;
    }

    PARSER_DEBUG("Opening '(' at line %d, col %d\n", str->line, str->col);
    str->pos++;
    str->col++;

    char* token = ReadToken (str);
    if (!token)
    {
        fprintf(stderr, "Error: expected token\n");
        return NULL;
    }

    Node* node = CreateNodeFromToken (token);
    free (token);

    if (!node)
    {

        SkipSpaces(str);
        if (str->input[str->pos] == ')')
        {
            str->pos++;
            str->col++;
            return NULL;
        }
        fprintf (stderr, "Error: failed to create node\n");
        return NULL;
    }

    PARSER_DEBUG("Parsing left child for node type %d\n", node->type);
    node->left = ParseExpression (str);

    PARSER_DEBUG("Parsing right child for node type %d\n", node->type);
    node->right = ParseExpression (str);

    SkipSpaces (str);

    if (str->input[str->pos] != ')')
    {
        fprintf (stderr, "Error: expected ')' at line %d, col %d\n", str->line, str->col);
        FreeTree (node);
        return NULL;
    }

    PARSER_DEBUG("Closing ')' at line %d, col %d\n", str->line, str->col);
    str->pos++;
    str->col++;

    return node;
}

Node* ParseLispAST (const char* str)
{
    if (!str || !*str) return NULL;

    ParserState state;
    CtorParser (&state, str);

    PARSER_DEBUG("=== Starting LISP Parser ===\n");
    Node* result = ParseExpression (&state);
    PARSER_DEBUG("=== Parser Finished ===\n");

    SkipSpaces (&state);

    if (state.input[state.pos] != '\0')
    {
        fprintf(stderr, "Warning: unparsed characters: %str\n", state.input + state.pos);
    }

    return result;
}
