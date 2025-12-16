#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

const int MAX_SIZE_NUM = 50;
const int MAX_STR_SIZE = 1000;

enum MyTokenType
{
    TOK_DECLARE,        // "Здравия_желаю_товарищ" - начало объявления
    TOK_TYPE_INT,       // "старшина" - тип int
    TOK_TYPE_CHAR,      // "рядовой" - тип char
    TOK_TYPE_DOUBLE,    // "прапорщик" - тип double

    TOK_IF,             // "Приготовиться_к_исполнению_по_получении_приказа" - if
    TOK_WHILE,          // "Исполнять_пока_получите_приказа" - while
    TOK_RETURN,         // "вольно" - return

    TOK_PLUS,           // "Включить_в_состав" - сложение (+)
    TOK_MINUS,          // "Исключить_из_состава" - вычитание (-)
    TOK_MULTIPLY,       // "усилить" - умножение (*)
    TOK_DIVIDE,         // "расформировать_до" - деление (/)

    TOK_EQ,             // "соответствует" - равно (==)
    TOK_NE,             // "не_соответствует" - не равно (!=)
    TOK_GT,             // "превосходит_норму" - больше (>)
    TOK_LT,             // "не_превосходит_норму" - меньше (<)

    TOK_ASSIGN,         // "Назначить" - присваивание (=)

    TOK_IDENTIFIER,     // Имена переменных/функций
    TOK_NUMBER,         // Числа

    TOK_SEMICOLON,      // ";"
    TOK_LPAREN,         // "("
    TOK_RPAREN,         // ")"
    TOK_LBRACE,         // "{"
    TOK_RBRACE,         // "}"
    TOK_COMMA,          // ","

    TOK_COMMENT,        // "Для_служебного_пользования"

    TOK_EOF,            // Конец файла
    TOK_UNKNOWN
};

struct Token
{
    MyTokenType type;
    union
    {
        double number;
        char* identifier;
    } value;
};

struct KeyWordToken
{
    MyTokenType token_type;
    const char* token_string;
};

typedef struct KeyWordToken KeywordToken;

struct Lexer
{
    const char* source;
    const char* current;
    Token* tokens;
    int line;
    int column;
    int capacity;
    int count;
};

typedef struct Lexer Lexer;

bool IsAtEnd (const Lexer* lexer);
char Peek (const Lexer* lexer);
char PeekNext (const Lexer* lexer);
void Advance (Lexer* lexer);
bool AddToken (Lexer* lexer, MyTokenType type, const char* value_start, int value_length);
bool ScanNumber (Lexer* lexer);
bool ScanIdentifier (Lexer* lexer);
bool ScanSymbol (Lexer* lexer);
char* ReadFile (const char* filename);
long GetFileSize (FILE* file);

Lexer* CtorLexer (const char* source_code);
void DtorLexer (Lexer* lexer);
bool LexerScanTokens (Lexer* lexer);

Token* LexerGetTokens (const Lexer* lexer);
int LexerGetTokenCount (const Lexer* lexer);

const char* TokenTypeToString (MyTokenType type);
void LexerPrintTokens (const Lexer* lexer);

Token* LexerOld (const char* source_code, int* token_count);
void FreeTokens (Token* tokens, int token_count);

static bool IsRussianLetter(char c);
static bool IsAlphaRu (char c);
static bool IsAlnumRu (char c);
static bool CanBeIdentifierCharRu (char c);


#endif
