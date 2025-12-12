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
    // Ключевые слова для объявлений
    TOK_DECLARE,        // "Здравия_желаю_товарищ" - начало объявления
    TOK_TYPE_INT,       // "старшина" - тип int
    TOK_TYPE_CHAR,      // "рядовой" - тип char
    TOK_TYPE_DOUBLE,    // "прапорщик" - тип double

    // Управляющие конструкции
    TOK_IF,             // "Приготовиться_к_исполнению_по_получении_приказа" - if
    TOK_RETURN,         // "вольно" - return

    // Арифметические операторы (заменены словами)
    TOK_PLUS,           // "Включить_в_состав" - сложение (+)
    TOK_MINUS,          // "Исключить_из_состава" - вычитание (-)
    TOK_MULTIPLY,       // "усилить" - умножение (*)
    TOK_DIVIDE,         // "расформировать_до" - деление (/)

    // Операторы сравнения (заменены словами)
    TOK_EQ,             // "соответствует" - равно (==)
    TOK_NE,             // "не_соответствует" - не равно (!=)
    TOK_GT,             // "превосходит_норму" - больше (>)
    TOK_LT,             // "не_превосходит_норму" - меньше (<)

    // Оператор присваивания
    TOK_ASSIGN,         // "Назначить" - присваивание (=)

    // Идентификаторы и константы
    TOK_IDENTIFIER,     // Имена переменных/функций (например: x, y, myVar)
    TOK_NUMBER,         // Числа (например: 42, 3.14)

    // Символьные токены (оставим как символы)
    TOK_SEMICOLON,      // ";" - конец инструкции
    TOK_LPAREN,         // "(" - открывающая круглая скобка
    TOK_RPAREN,         // ")" - закрывающая круглая скобка
    TOK_LBRACE,         // "{" - открывающая фигурная скобка
    TOK_RBRACE,         // "}" - закрывающая фигурная скобка
    TOK_COMMA,          // "," - разделитель аргументов

    // Комментарии
    TOK_COMMENT,        // "Для_служебного_пользования" - начало комментария

    // Специальные токены
    TOK_EOF,            // Конец файла (не виден в тексте)
    TOK_UNKNOWN         // Неизвестный символ (ошибка)
};

typedef struct
{
    MyTokenType type;
    union
    {
        double number;
        char* identifier;
    } value;
}Token;

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
