#include "lexical_analysis.h"

/*

1 обьявление : Здравия желаю, товарищ
2 определение типа : (int старшина, char рядовой, double прапорщик)
3 ( -- Извините, к вам тут (и все то что передается в скобках)
4 ) -- Разрешите идти
5 { -- Боевая задача
6 } -- Отставить
7 if -- Приготовиться к исполнению по получении приказа
8 return -- вольно
9 + -- Включить в состав
10 - -- Исключить из состава
11 * -- услилить
12 / -- расформировать до
13 == -- соответствует
14 != -- не соответствует
15 > -- превосходит норму
15 < -- не превосходит норму
16 = -- Назначить
17 //(комментарий) -- Для служебного пользования
18 ; -- Выполнять!
19 = -- Назначить
20 while -- Исполнять_пока_получите_приказа
*/

static KeywordToken keyword_tokens[] =
{
    {TOK_DECLARE,       "Здравия_желаю_товарищ"},
    {TOK_TYPE_INT,      "старшина"},
    {TOK_TYPE_CHAR,     "рядовой"},
    {TOK_TYPE_DOUBLE,   "прапорщик"},

    {TOK_IF,            "Приготовиться_к_исполнению_по_получении_приказа"},
    {TOK_WHILE,         "Исполнять_пока_не_получите_приказа"},
    {TOK_RETURN,        "вольно"},

    {TOK_PLUS,          "Включить_в_состав"},
    {TOK_MINUS,         "Исключить_из_состава"},
    {TOK_MULTIPLY,      "усилить"},
    {TOK_DIVIDE,        "расформировать_до"},

    {TOK_EQ,            "соответствует"},
    {TOK_NE,            "не_соответствует"},
    {TOK_GT,            "превосходит_норму"},
    {TOK_LT,            "не_превосходит_норму"},

    {TOK_ASSIGN,        "Назначить"},

    {TOK_COMMENT,       "Для_служебного_пользования"},
    {TOK_UNKNOWN,       NULL}
};

char* ReadFile (const char* filename)
{
    FILE* file = fopen (filename, "r");
    if (!file)
    {
        fprintf (stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    long size = GetFileSize (file);

    char* buffer = (char*) calloc (size + 1, sizeof(char));
    if (!buffer)
    {
        fclose (file);
        return NULL;
    }

    fread (buffer, 1, size, file);
    buffer[size] = '\0';
    fclose (file);

    return buffer;
}

long GetFileSize (FILE* file)
{
    if (!file) return -1;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    return size;
}

static const char* skip_phrases[] =
{
    "солдат_в_подчинение",
    "Солдат_в_подчинение",
    "по_приказу_вышепоставленных_органов",
    "По_приказу_вышепоставленных_органов",
    "Товарищ",
    "товарищ",
    "Докладываю",
    "докладываю",
    "Рапортую",
    "рапортую",
    "Доложил",
    "доложил",
    "что",
    "необходимо",
    "вас",
    "Вас",
    "Всех_солдат_что_у",
    "всех_солдат_что_у",
    NULL
};

MyTokenType FindTokenByString (const char* str)
{
    #ifdef DEBUG
        printf("DEBUG: Ищем токен для строки: '%s' (длина: %d)\n", str, (int)strlen(str));
    #endif

    for (size_t i = 0; i < sizeof (keyword_tokens)/ sizeof (keyword_tokens[0]); i++)
    {
        if (keyword_tokens[i].token_string == NULL)
            break;

        #ifdef DEBUG
            printf("DEBUG: Сравниваем с: '%s'\n", keyword_tokens[i].token_string);
        #endif

        if (strcmp (keyword_tokens[i].token_string, str) == 0)
        {
            #ifdef DEBUG
                printf("DEBUG: Нашли! Тип: %d\n", keyword_tokens[i].token_type);
            #endif

            return keyword_tokens[i].token_type;
        }
    }
    #ifdef DEBUG
        printf("DEBUG: Не нашли, возвращаем IDENTIFIER\n");
    #endif
    return TOK_IDENTIFIER;
}

char Peek (const Lexer* lexer)
{
    return lexer->current[0];
}

char PeekNext (const Lexer* lexer)
{
    if (lexer->current[1] == '\0') return '\0';
    return lexer->current[1];
}

bool IsAtEnd (const Lexer* lexer)
{
    return lexer->current[0] == '\0';
}

void Advance (Lexer* lexer)
{
    if (Peek (lexer) == '\n')
    {
        lexer->line++;
        lexer->column = 1;
    }
    else
        lexer->column++;

    lexer->current++;
}

bool AddToken (Lexer* lexer, MyTokenType type, const char* value_start, int value_length)
{
    if (lexer->count >= lexer->capacity)
    {
        lexer->capacity *= 2;
        Token* new_tokens = (Token*)realloc (lexer->tokens, lexer->capacity * sizeof(Token));

        if (!new_tokens)
            return false;

        lexer->tokens = new_tokens;
    }

    lexer->tokens[lexer->count].type = type;

    if (type == TOK_IDENTIFIER)
    {
        lexer->tokens[lexer->count].value.identifier = (char*) calloc (1, value_length + 1);

        if (!lexer->tokens[lexer->count].value.identifier)
            return false;

        strncpy (lexer->tokens[lexer->count].value.identifier, value_start, value_length);
        lexer->tokens[lexer->count].value.identifier[value_length] = '\0';
    }
    else if (type == TOK_NUMBER)
    {
        char buffer[MAX_SIZE_NUM] = {};
        strncpy (buffer, value_start, value_length);
        lexer->tokens[lexer->count].value.number = atof (buffer);
    }
    else
    {
        lexer->tokens[lexer->count].value.identifier = NULL;
    }

    lexer->count++;
    return true;
}

bool ScanNumber (Lexer* lexer)
{
    const char* start = lexer->current;

    while (isdigit (Peek (lexer)))
        Advance (lexer);

    if (Peek (lexer) == '.' && isdigit (PeekNext (lexer)))
    {
        Advance (lexer);

        while (isdigit (Peek (lexer)))
            Advance (lexer);
    }

    int length = lexer->current - start;

    return AddToken (lexer, TOK_NUMBER, start, length);
}

bool ScanIdentifier (Lexer* lexer)
{
    const char* start = lexer->current;

    Advance (lexer);

    while (CanBeIdentifierCharRu (Peek (lexer)))
        Advance(lexer);

    int length = lexer->current - start;

    char buffer[MAX_STR_SIZE] = {};
    if (length >= MAX_STR_SIZE - 1) length = MAX_STR_SIZE - 2;
    strncpy(buffer, start, length);
    buffer[length] = '\0';


    if (strcmp (buffer, "Для_служебного_пользования") == 0)
    {

        while (!IsAtEnd (lexer) && Peek(lexer) != '\n')
            Advance (lexer);

        return true;
    }

    for (int i = 0; skip_phrases[i] != NULL; i++)
    {
        if (strcmp (buffer, skip_phrases[i]) == 0)
        {
            return true;
        }
    }

    MyTokenType type = FindTokenByString (buffer);

    return AddToken (lexer, type, start, length);
}

bool ScanSymbol (Lexer* lexer)
{
    char current_char = Peek (lexer);
    MyTokenType type = TOK_UNKNOWN;

    switch (current_char)
    {
        case ';': type = TOK_SEMICOLON; break;
        case '(': type = TOK_LPAREN; break;
        case ')': type = TOK_RPAREN; break;
        case '{': type = TOK_LBRACE; break;
        case '}': type = TOK_RBRACE; break;
        case ',': type = TOK_COMMA; break;
        default: return false;
    }

    if (!AddToken (lexer, type, lexer->current, 1))
        return false;

    Advance (lexer);
    return true;
}

Lexer* CtorLexer (const char* source_code)
{
    Lexer* lexer = (Lexer*) calloc (1, sizeof (Lexer));
    if (!lexer) return NULL;

    lexer->source = source_code;
    lexer->current = source_code;
    lexer->line = 1;
    lexer->column = 1;

    lexer->capacity = 64;
    lexer->count = 0;
    lexer->tokens = (Token*) calloc (lexer->capacity, sizeof(Token));

    if (!lexer->tokens)
    {
        free(lexer);
        return NULL;
    }

    return lexer;
}

void DtorLexer (Lexer* lexer)
{
    if (!lexer) return;


    for (int i = 0; i < lexer->count; i++)
    {
        if (lexer->tokens[i].type == TOK_IDENTIFIER && lexer->tokens[i].value.identifier)
            free (lexer->tokens[i].value.identifier);
    }

    if (lexer->tokens)
        free(lexer->tokens);

    free(lexer);
}

bool LexerScanTokens (Lexer* lexer)
{
    if (!lexer) return false;

    while (!IsAtEnd (lexer))
    {
        if (isspace (Peek (lexer)))
        {
            if (Peek (lexer) == '\n')
            {
                lexer->line++;
                lexer->column = 1;
            }
            Advance (lexer);
            continue;
        }

        if (Peek(lexer) == '\t')
        {
            lexer->column += 4;
            Advance (lexer);
            continue;
        }

        if (Peek(lexer) == '\r')
        {
            Advance (lexer);
            continue;
        }


        if (isdigit ((unsigned char) Peek (lexer)))
        {
            if (!ScanNumber (lexer))
                return false;

            continue;
        }

        if (IsAlphaRu (Peek (lexer)) || Peek (lexer) == '_')
        {
            #ifdef DEBUG
                printf("DEBUG: Начало идентификатора на символе '%c' (код: %d)\n",
                       Peek (lexer), (unsigned char) Peek (lexer));
            #endif

            if (!ScanIdentifier (lexer) )
                return false;

            continue;
        }

        if (ScanSymbol (lexer))
            continue;
        #ifdef DEBUG
            printf("DEBUG: Неизвестный символ: '%c' (код: %d), line: %d, col: %d\n",
                Peek(lexer), (unsigned char)Peek(lexer), lexer->line, lexer->column);
            printf("DEBUG: Контекст: '%.10s'\n", lexer->current);
        #endif

        fprintf (stderr, "Ошибка лексического анализа (строка %d, столбец %d): неизвестный символ '%c' (код: %d)\n",
                 lexer->line, lexer->column, Peek(lexer), (unsigned char)Peek(lexer));
        Advance (lexer);

        #ifdef DEBUG
        if (!ScanSymbol (lexer))
        {
            printf ("DEBUG UNKNOWN CHAR: '%c' (ASCII: %d, hex: 0x%02x) at line %d, col %d\n",
                   Peek(lexer),
                   (unsigned char)Peek(lexer),
                   (unsigned char)Peek(lexer),
                   lexer->line, lexer->column);

            fprintf (stderr, "Ошибка лексического анализа (строка %d, столбец %d): неизвестный символ '%c' (код: %d)\n",
                    lexer->line, lexer->column, Peek(lexer), (unsigned char)Peek(lexer));
            Advance (lexer);
            continue;
        }

        char c = Peek(lexer);
        printf("\n=== DEBUG UNKNOWN CHARACTER DETECTED ===\n");
        printf("Позиция: строка %d, столбец %d\n", lexer->line, lexer->column);
        printf("Символ: '%c'\n", c);
        printf("ASCII код: %d\n", (unsigned char)c);
        printf("Hex: 0x%02x\n", (unsigned char)c);

        // Показать контекст
        printf("Контекст (20 символов): '");
        const char* context = lexer->current;
        for (int i = 0; i < 20 && context[i] != '\0'; i++) {
            if (context[i] == '\n') printf("\\n");
            else if (context[i] == '\r') printf("\\r");
            else if (context[i] == '\t') printf("\\t");
            else printf("%c", context[i]);
        }
        printf("'\n");

        // Показать предыдущие и следующие токены
        printf("Текущий индекс токенов: %d\n", lexer->count);
        if (lexer->count > 0) {
            printf("Предыдущий токен: ");
            Token* prev = &lexer->tokens[lexer->count - 1];
            printf("Тип: %s, ", TokenTypeToString(prev->type));
            if (prev->type == TOK_IDENTIFIER && prev->value.identifier)
                printf("Значение: '%s'\n", prev->value.identifier);
            else if (prev->type == TOK_NUMBER)
                printf("Значение: %g\n", prev->value.number);
            else
                printf("\n");
        }
        printf("=== END DEBUG ===\n\n");
        // === КОНЕЦ ДОБАВЛЕННОГО КОДА ===

        printf("=== DEBUG UNKNOWN CHAR ===\n");
        printf("Символ: '%c'\n", Peek(lexer));
        printf("ASCII код: %d\n", (unsigned char)Peek(lexer));
        printf("Hex: 0x%02x\n", (unsigned char)Peek(lexer));
        printf("Позиция: строка %d, столбец %d\n", lexer->line, lexer->column);

        printf("Контекст: '");
        for (int i = 0; i < 20 && lexer->current[i] != '\0'; i++)
        {
            if (lexer->current[i] == '\n') printf("\\n");
            else if (lexer->current[i] == '\r') printf("\\r");
            else if (lexer->current[i] == '\t') printf("\\t");
            else printf("%c", lexer->current[i]);
        }
        printf("'\n");
        printf("=== КОНЕЦ DEBUG ===\n");

        fprintf (stderr, "Ошибка лексического анализа (строка %d, столбец %d): неизвестный символ '%c' (код: %d)\n",
                 lexer->line, lexer->column, Peek(lexer), (unsigned char)Peek(lexer));
        Advance (lexer);

        #endif
    }

    return AddToken (lexer, TOK_EOF, "", 0);
}

static bool IsRussianLetter (char c)
{
    unsigned char uc = (unsigned char) c;

    return (uc >= 192 && uc <= 223) ||    // А-Я
           (uc >= 224 && uc <= 255) ||    // а-я
           (uc == 168) || (uc == 184);    // Ё, ё
}

static bool IsAlphaRu (char c)
{
    return isalpha (c) || IsRussianLetter (c);
}

static bool IsAlnumRu (char c)
{
    return isalnum (c) || IsRussianLetter (c);
}

static bool CanBeIdentifierCharRu (char c)
{
    return isalnum (c) || c == '_' || IsRussianLetter (c);
}

Token* LexerGetTokens (const Lexer* lexer)
{
    if (!lexer) return NULL;
    return lexer->tokens;
}

int LexerGetTokenCount (const Lexer* lexer)
{
    if (!lexer) return 0;
    return lexer->count;
}

const char* TokenTypeToString (MyTokenType type)
{
    switch (type)
    {
        case TOK_DECLARE:       return "TOK_DECLARE";
        case TOK_TYPE_INT:      return "TOK_TYPE_INT";
        case TOK_TYPE_CHAR:     return "TOK_TYPE_CHAR";
        case TOK_TYPE_DOUBLE:   return "TOK_TYPE_DOUBLE";
        case TOK_IF:            return "TOK_IF";
        case TOK_RETURN:        return "TOK_RETURN";
        case TOK_PLUS:          return "TOK_PLUS";
        case TOK_MINUS:         return "TOK_MINUS";
        case TOK_MULTIPLY:      return "TOK_MULTIPLY";
        case TOK_DIVIDE:        return "TOK_DIVIDE";
        case TOK_EQ:            return "TOK_EQ";
        case TOK_NE:            return "TOK_NE";
        case TOK_GT:            return "TOK_GT";
        case TOK_LT:            return "TOK_LT";
        case TOK_ASSIGN:        return "TOK_ASSIGN";
        case TOK_IDENTIFIER:    return "TOK_IDENTIFIER";
        case TOK_NUMBER:        return "TOK_NUMBER";
        case TOK_SEMICOLON:     return "TOK_SEMICOLON";
        case TOK_LPAREN:        return "TOK_LPAREN";
        case TOK_RPAREN:        return "TOK_RPAREN";
        case TOK_LBRACE:        return "TOK_LBRACE";
        case TOK_RBRACE:        return "TOK_RBRACE";
        case TOK_COMMA:         return "TOK_COMMA";
        case TOK_COMMENT:       return "TOK_COMMENT";
        case TOK_EOF:           return "TOK_EOF";
        case TOK_UNKNOWN:       return "TOK_UNKNOWN";
        default:                return "UNKNOWN_TOKEN_TYPE";
    }
}

void LexerPrintTokens (const Lexer* lexer)
{
    if (!lexer)
    {
        printf("Лексер не инициализирован\n");
        return;
    }

    printf ("=== ЛЕКСИЧЕСКИЙ АНАЛИЗ: найдено %d токенов ===\n", lexer->count);
    printf ("%-5s %-30s %-20s %-10s\n", "№", "Тип токена", "Значение", "Позиция");
    printf ("-------------------------------------------------------------\n");

    for (int i = 0; i < lexer->count; i++)
    {
        const Token* token = &lexer->tokens[i];

        printf("%-5d %-30s ", i, TokenTypeToString (token->type));

        switch (token->type)
        {
            case TOK_NUMBER:
                printf("%-20g", token->value.number);
                break;

            case TOK_IDENTIFIER:
                if (token->value.identifier)
                    printf ("%-20s", token->value.identifier);
                else
                    printf ("%-20s", "(null)");

                break;

            case TOK_EOF:
                printf ("%-20s", "END OF FILE");
                break;

            default:
                printf("%-20s", "—");
                break;
        }


        printf (" line:%d\n", lexer->line);
    }
}

Token* LexerOld (const char* source_code, int* token_count)
{
    Lexer* lexer = CtorLexer (source_code);

    if (!lexer)
    {
        *token_count = 0;
        return NULL;
    }

    if (!LexerScanTokens (lexer))
    {
        DtorLexer(lexer);
        *token_count = 0;
        return NULL;
    }

    Token* tokens = lexer->tokens;
    *token_count = lexer->count;

    lexer->tokens = NULL;
    lexer->count = 0;
    DtorLexer (lexer);

    return tokens;
}

void FreeTokens (Token* tokens, int token_count)
{
    if (!tokens) return;

    for (int i = 0; i < token_count; i++)
    {
        if (tokens[i].type == TOK_IDENTIFIER && tokens[i].value.identifier)
            free(tokens[i].value.identifier);
    }
    free (tokens);
}
