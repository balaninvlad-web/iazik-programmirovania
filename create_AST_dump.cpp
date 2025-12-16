#include "create_AST_dump.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

const int MAX_COMMAND_LENGTH = 200;

void SafePrintNodeType (FILE* dot_file, NodeType type)
{
    const char* type_str = NodeTypeToString (type);
    fprintf(dot_file, "%s", type_str);
}

void CreateGraphvizDump (Node* root, const char* filename)
{
    assert (filename);

    FILE* dot_file = fopen (filename, "w");
    if (!dot_file)
    {
        fprintf (stderr, "Ошибка создания DOT файла: %s\n", filename);
        return;
    }

    CreateGraphvizHeader (dot_file);

    if (!root)
    {
        fprintf (dot_file, "    empty [label=\"EMPTY TREE\\nRoot: NULL\", "
                           "shape=box, color=red, fontcolor=white];\n");
    }
    else
    {
        CreateGraphvizNodes (dot_file, root);
        CreateGraphvizEdges (dot_file, root);
    }

    fprintf (dot_file, "}\n");
    fclose (dot_file);

    GenerateImage (filename);
}

void CreateGraphvizHeader (FILE* dot_file)
{
    fprintf (dot_file, "digraph AST {\n");
    fprintf (dot_file, "    charset=\"latin1\";\n");
    fprintf (dot_file, "    bgcolor=\"#001f29\"\n");
    fprintf (dot_file, "    rankdir = TB\n");
    fprintf (dot_file, "    nodesep = 0.5;\n");
    fprintf (dot_file, "    ranksep = 0.7;\n");
    fprintf (dot_file, "    node [shape=plaintext, style=filled, fontname=\"Arial\"];\n");
    fprintf (dot_file, "    edge [fontname=\"Arial\"];\n\n");
}

void CreateGraphvizNodes(FILE* dot_file, Node* node)
{
    if (!node) return;

    const char* fillcolor = "";
    const char* color = "black";

    if (node->type == NODE_NUMBER) {
        fillcolor = "#445c00";
        color = "#fdfdfd";
    } else if (node->type == NODE_VARIABLE || node->type == NODE_VAR_DECL) {
        fillcolor = "#2799a0";
        color = "#fdfdfd";
    } else if (node->type == NODE_FUNC_DECL || node->type == NODE_FUNC_CALL) {
        fillcolor = "#5f3035";
        color = "#fdfdfd";
    } else if (node->type == NODE_IF || node->type == NODE_RETURN) {
        fillcolor = "#8a2be2";
        color = "#fdfdfd";
    } else if (node->type >= NODE_ADD && node->type <= NODE_LT) {
        fillcolor = "#9b59b6";
        color = "#fdfdfd";
    } else if (node->type == NODE_SEQUENCE) {
        fillcolor = "#3498db";
        color = "#fdfdfd";
    } else if (node->type == NODE_EMPTY) {
        fillcolor = "#95a5a6";
        color = "#fdfdfd";
    } else {
        fillcolor = "#2c3e50";
        color = "#fdfdfd";
    }

    fprintf (dot_file, "    node%p [label=<<TABLE BORDER='1' CELLBORDER='1' CELLSPACING='0'>\n", (void*)node);
    fprintf (dot_file, "        <TR><TD COLSPAN='2'><B>%s</B></TD></TR>\n", NodeTypeToString (node->type));
    fprintf (dot_file, "        <TR><TD COLSPAN='2'>addr: %p</TD></TR>\n", (void*)node);
    if (node->priority > 0)
    {
        fprintf (dot_file, "        <TR><TD COLSPAN='2'>priority: %d</TD></TR>\n", node->priority);
    }
    switch (node->type)
    {
        case NODE_NUMBER:
            fprintf (dot_file, "        <TR><TD COLSPAN='2'>value: %g</TD></TR>\n", node->data.number_value);
            break;

        case NODE_VARIABLE:
        case NODE_VAR_DECL:
        case NODE_FUNC_DECL:
        case NODE_FUNC_CALL:
            if (node->data.string_value)
            {
                fprintf (dot_file, "        <TR><TD COLSPAN='2'>name: <FONT COLOR='yellow'>");
                SafePrintString (dot_file, node->data.string_value);
                fprintf (dot_file, "</FONT></TD></TR>\n");
            }
            if (node->type == NODE_VAR_DECL || node->type == NODE_FUNC_DECL) {
                fprintf (dot_file, "        <TR><TD COLSPAN='2'>type: %d</TD></TR>\n", node->data.type_value);
            }
            break;
        case NODE_TYPE_INT:
            fprintf (dot_file, "        <TR><TD COLSPAN='2'>type: int</TD></TR>\n");
            break;

        case NODE_TYPE_CHAR:
            fprintf (dot_file, "        <TR><TD COLSPAN='2'>type: char</TD></TR>\n");
            break;

        case NODE_TYPE_DOUBLE:
            fprintf (dot_file, "        <TR><TD COLSPAN='2'>type: double</TD></TR>\n");
            break;

        default:
            break;
    }

    fprintf (dot_file, "        <TR><TD>left</TD>");
    fprintf (dot_file, "<TD>right</TD></TR>\n");
    fprintf (dot_file, "        <TR><TD PORT='left'>%p</TD><TD PORT='right'>%p</TD></TR>\n",
            (void*)node->left, (void*)node->right);

    fprintf (dot_file, "    </TABLE>>, fillcolor=\"%s\", color=\"%s\", fontcolor=\"%s\"];\n\n",
            fillcolor, color, color);

    CreateGraphvizNodes (dot_file, node->left);
    CreateGraphvizNodes (dot_file, node->right);
}

void CreateGraphvizEdges (FILE* dot_file, Node* node)
{
    if (!node) return;

    if (node->left)
    {
        fprintf (dot_file, "    node%p:left -> node%p [color=\"#adebff\", penwidth=2, label=\"LEFT\", "
                "fontcolor=\"#adebff\", fontsize=13, arrowsize=0.8];\n",
                (void*)node, (void*)node->left);
    }

    if (node->right)
    {
        fprintf (dot_file, "    node%p:right -> node%p [color=\"#ffadb1\", penwidth=2, label=\"RIGHT\", "
                "fontcolor=\"#ffadb1\", fontsize=13, arrowsize=0.8];\n",
                (void*)node, (void*)node->right);
    }


    CreateGraphvizEdges (dot_file, node->left);
    CreateGraphvizEdges (dot_file, node->right);
}

void GenerateImage(const char* dot_filename)
{
    static int image_counter = 1;
    char command[MAX_COMMAND_LENGTH] = "";
    char output_filename[MAX_COMMAND_LENGTH] = "";

    #ifdef _WIN32
        system ("if not exist ast_images mkdir ast_images");
    #else
        system ("mkdir -p ast_images 2>/dev/null");
    #endif

    snprintf (output_filename, sizeof(output_filename), "ast_images/ast_dump%d.svg", image_counter);

    snprintf (command, sizeof(command), "dot -Tsvg -Gcharset=latin1 %s -o %s", dot_filename, output_filename);

    int result = system (command);
    if (result != 0)
    {
        printf("Ошибка генерации SVG. Пробуем без charset...\n");
        snprintf (command, sizeof(command), "dot -Tsvg %s -o %s", dot_filename, output_filename);
        system (command);
    }

    printf ("Граф сохранен в: ast_images/ast_dump%d.svg\n", image_counter);
    image_counter++;
}

static FILE* html_file = NULL;
static int html_dump_counter = 1;

void CreateHtmlDump (Node* tree, const char* func, const char* reason, ...)
{
    if (!html_file)
    {
        html_file = fopen ("ast_dumps.html", "w");
        if (!html_file)
        {
            fprintf (stderr, "Ошибка создания HTML файла\n");
            return;
        }

        fprintf (html_file, "<!DOCTYPE html>\n");
        fprintf (html_file, "<html lang='ru'>\n");
        fprintf (html_file, "<head>\n");
        fprintf (html_file, "<meta charset='UTF-8'>\n");
        fprintf (html_file, "<title>AST Dumps</title>\n");
        fprintf (html_file, "<style>\n");
        fprintf (html_file, "  body { background-color: #001f29; color: #ffffff; }\n");
        fprintf (html_file, "  .dump { border: 2px solid #0099cc; padding: 15px; margin: 10px; }\n");
        fprintf (html_file, "  h1, h2, h3 { color: #00ccff; }\n");
        fprintf (html_file, "  img { max-width: 100%%; height: auto; margin: 10px; }\n");
        fprintf (html_file, "</style>\n");
        fprintf (html_file, "</head>\n");
        fprintf (html_file, "<body>\n");
        fprintf (html_file, "<h1>AST Dumps</h1>\n");
    }

    fprintf (html_file, "<div class='dump'>\n");
    fprintf (html_file, "<h2>Dump #%d</h2>\n", html_dump_counter);
    fprintf (html_file, "<p><b>Функция:</b> %s</p>\n", func);

    va_list args = {};
    va_start (args, reason);
    char formatted_reason[MAX_COMMAND_LENGTH] = {};
    vsnprintf (formatted_reason, sizeof(formatted_reason), reason, args);
    va_end (args);

    fprintf (html_file, "<p><b>Причина:</b> %s</p>\n", formatted_reason);

    char dot_filename[MAX_COMMAND_LENGTH] = {};
    char svg_filename[MAX_COMMAND_LENGTH] = {};

    snprintf (dot_filename, sizeof (dot_filename), "dump_%d.dot", html_dump_counter);
    snprintf (svg_filename, sizeof (svg_filename), "ast_images/dump_%d.svg", html_dump_counter);

    CreateGraphvizDump (tree, dot_filename);

    fprintf (html_file, "<h3>Визуализация AST</h3>\n");
    fprintf (html_file, "<img src='%s' alt='AST Dump #%d'>\n", svg_filename, html_dump_counter);

    fprintf(html_file, "</div>\n");
    fflush(html_file);

    html_dump_counter++;
}

void CloseHtmlFile(void)
{
    if (html_file)
    {
        fprintf (html_file, "</body>\n</html>\n");
        fclose (html_file);
        html_file = NULL;
        printf ("HTML файл дампов сохранен: ast_dumps.html\n");
    }
}

const char* NodeTypeToString (NodeType type)
{
     switch ((int)type)
     {
        case 0:  return "NODE_EMPTY";
        case 1:  return "NODE_SEQUENCE";
        case 2:  return "NODE_NUMBER";
        case 3:  return "NODE_VARIABLE";
        case 4:  return "NODE_ADD";
        case 5:  return "NODE_SUB";
        case 6:  return "NODE_MUL";
        case 7:  return "NODE_DIV";
        case 8:  return "NODE_EQ";
        case 9:  return "NODE_NE";
        case 10: return "NODE_GT";
        case 11: return "NODE_LT";
        case 12: return "NODE_ASSIGNMENT";
        case 13: return "NODE_VAR_DECL";
        case 14: return "NODE_FUNC_DECL";
        case 15: return "NODE_FUNC_CALL";
        case 16: return "NODE_IF";
        case 17: return "NODE_RETURN";
        case 18: return "NODE_TYPE_INT";
        case 19: return "NODE_TYPE_CHAR";
        case 20: return "NODE_TYPE_DOUBLE";
        default: return "UNKNOWN";
    }
}

static const char* TransliterateChar(unsigned char c)
{
    static const char* translit[256] = {NULL};
    static int initialized = 0;

    if (!initialized)
    {
        for (int i = 0; i < 256; i++) translit[i] = NULL;

        // Заглавные русские буквы (CP1251)
        translit[0xC0] = "A";   // А
        translit[0xC1] = "B";   // Б
        translit[0xC2] = "V";   // В
        translit[0xC3] = "G";   // Г
        translit[0xC4] = "D";   // Д
        translit[0xC5] = "E";   // Е
        translit[0xA8] = "Yo";  // Ё
        translit[0xC6] = "Zh";  // Ж
        translit[0xC7] = "Z";   // З
        translit[0xC8] = "I";   // И
        translit[0xC9] = "J";   // Й
        translit[0xCA] = "K";   // К
        translit[0xCB] = "L";   // Л
        translit[0xCC] = "M";   // М
        translit[0xCD] = "N";   // Н
        translit[0xCE] = "O";   // О
        translit[0xCF] = "P";   // П
        translit[0xD0] = "R";   // Р
        translit[0xD1] = "S";   // С
        translit[0xD2] = "T";   // Т
        translit[0xD3] = "U";   // У
        translit[0xD4] = "F";   // Ф
        translit[0xD5] = "H";   // Х
        translit[0xD6] = "C";   // Ц
        translit[0xD7] = "Ch";  // Ч
        translit[0xD8] = "Sh";  // Ш
        translit[0xD9] = "Sch"; // Щ
        translit[0xDA] = "\"";  // Ъ
        translit[0xDB] = "Y";   // Ы
        translit[0xDC] = "'";   // Ь
        translit[0xDD] = "E";   // Э
        translit[0xDE] = "Yu";  // Ю
        translit[0xDF] = "Ya";  // Я

        // Строчные русские буквы
        translit[0xE0] = "a";   // а
        translit[0xE1] = "b";   // б
        translit[0xE2] = "v";   // в
        translit[0xE3] = "g";   // г
        translit[0xE4] = "d";   // д
        translit[0xE5] = "e";   // е
        translit[0xB8] = "yo";  // ё
        translit[0xE6] = "zh";  // ж
        translit[0xE7] = "z";   // з
        translit[0xE8] = "i";   // и
        translit[0xE9] = "j";   // й
        translit[0xEA] = "k";   // к
        translit[0xEB] = "l";   // л
        translit[0xEC] = "m";   // м
        translit[0xED] = "n";   // н
        translit[0xEE] = "o";   // о
        translit[0xEF] = "p";   // п
        translit[0xF0] = "r";   // р
        translit[0xF1] = "s";   // с
        translit[0xF2] = "t";   // т
        translit[0xF3] = "u";   // у
        translit[0xF4] = "f";   // ф
        translit[0xF5] = "h";   // х
        translit[0xF6] = "c";   // ц
        translit[0xF7] = "ch";  // ч
        translit[0xF8] = "sh";  // ш
        translit[0xF9] = "sch"; // щ
        translit[0xFA] = "\"";  // ъ
        translit[0xFB] = "y";   // ы
        translit[0xFC] = "'";   // ь
        translit[0xFD] = "e";   // э
        translit[0xFE] = "yu";  // ю
        translit[0xFF] = "ya";  // я

        translit[0x2D] = "-";
        translit[0x5F] = "_";

        initialized = 1;
    }

    return translit[c];
}

void SafePrintString(FILE* dot_file, const char* str)
{
    if (!str)
    {
        fprintf(dot_file, "(null)");
        return;
    }

    for (int i = 0; str[i] != '\0'; i++)
    {
        unsigned char c = str[i];

        if (c >= 32 && c <= 126)
        {
            switch (c)
            {
                case '<': fprintf (dot_file, "&lt;"); break;
                case '>': fprintf (dot_file, "&gt;"); break;
                case '&': fprintf (dot_file, "&amp;"); break;
                case '"': fprintf (dot_file, "&quot;"); break;
                case '\'': fprintf (dot_file, "&#39;"); break;
                default: fputc (c, dot_file); break;
            }
        }
        else if ((c >= 0xC0 && c <= 0xDF) ||  // А-Я (кроме Ё)
                 (c >= 0xE0 && c <= 0xFF) ||  // а-я (кроме ё)
                 c == 0xA8 || c == 0xB8)
        {
            const char* translit = TransliterateChar (c);
            if (translit)
            {
                fprintf (dot_file, "%s", translit);
            }
            else
                fprintf (dot_file, "?");
        }
    }
}
