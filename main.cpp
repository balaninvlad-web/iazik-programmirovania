#include "lexical_analysis.h"
#include "tree_base.h"
#include "syntactic_analysis.h"
#include "create_AST_dump.h"

int main(int argc, char* argv[])
{
    char* test_program = NULL;

    if (argc  > 1)
    {
        char* filename = argv[1];
        printf ("Loading tree from file: %s\n", filename);

        test_program = ReadFile (filename);
        if (!test_program) printf ("ERORR: in ReadFile\n");

        printf ("Исходный код:\n");
        printf ("-------------------------------------------------\n");
        printf ("%s\n", test_program);
        printf ("-------------------------------------------------\n\n");
    }
    else
    {
        printf ("No input file specified. Creating default tree...\n");
        return 0;
    }

    Lexer* lexer = CtorLexer (test_program);
    if (lexer && LexerScanTokens (lexer))
    {
        LexerPrintTokens (lexer);
    }

    Getter* Getter = CtorGetter (lexer);
    if (!Getter)
    {
        printf ("Ошибка создания парсера\n");
        DtorLexer (lexer);
        return 1;
    }

    Node* Ast_root = GetProgram (Getter);

    if (Getter->error_count > 0)
    {
        printf("\nПарсер завершился с %d ошибками\n", Getter->error_count);
    }
    else if (Ast_root)
    {
        printf("\n=== АБСТРАКТНОЕ СИНТАКСИЧЕСКОЕ ДЕРЕВО ===\n");
        PrintTree(Ast_root, 0);

        FreeTree (Ast_root);

        CreateGraphvizDump (Ast_root, "ast_graph.dot");

        CreateHtmlDump (Ast_root, "main", "Парсинг завершен успешно");

        FreeTree (Ast_root);
    }
    else
    {
        printf ("\nПарсер вернул NULL без ошибок\n");
    }

    CloseHtmlFile ();
    DtorGetter (Getter);
    DtorLexer (lexer);
    printf ("\nПрограмма завершена успешно\n");
}

/*
МУСОРКА:

const char* test_program =
        "Здравия_желаю_товарищ старшина Яйцов () {\n"
        "    Товарищ старшина Хохлов Назначить 10 солдат_в_подчинение;\n"
        "    Товарищ старшина Петушаров Назначить 20 солдат_в_подчинение;\n"
        "    старшина Цыпочкин рапортую что по_приказу_вышепоставленных_органов необходимо Назначить Петушаров Включить_в_состав Хохлов;\n"
        "    Докладываю Приготовиться_к_исполнению_по_получении_приказа (что Цыпочкин превосходит_норму Хохлов) {\n"
        "        Товарищ Цыпочкин Рапортую что по_приказу_вышепоставленных_органов необходимо Назначить Цыпочкин расформировать_до Петушаров;\n"
        "    }\n"
        "вольно Цыпочкин;\n"
        "}\n"
        "Для_служебного_пользования Тестовый комментарий\n";

    printf ("=== ТЕСТ ЛЕКСИЧЕСКОГО АНАЛИЗАТОРА (ООП стиль) ===\n\n");
    printf ("Исходный код:\n");
    printf ("-------------------------------------------------\n");
    printf ("%s\n", test_program);
    printf ("-------------------------------------------------\n\n");

*/
