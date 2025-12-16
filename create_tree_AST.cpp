#include "tree_base.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static void DumpNewFormat (Node* node, FILE* file, int depth)
{
    if (!node)
    {
        for (int i = 0; i < depth; i++)
        fprintf (file, "  ");
        fprintf (file, "nil");
        return;
    }

    for (int i = 0; i < depth; i++)
    fprintf (file, "  ");
    fprintf (file, "( ");

    switch (node->type)
    {
        // Листья
        case NODE_NUMBER:
            fprintf(file, "%g", node->data.number_value);
            break;

        case NODE_VARIABLE:
            fprintf(file, "%s", node->data.string_value);
            break;

        // Операторы (в нижнем регистре)
        case NODE_SEQUENCE:      fprintf(file, ";"); break;
        case NODE_ASSIGNMENT:    fprintf(file, "="); break;
        case NODE_ADD:           fprintf(file, "+"); break;
        case NODE_SUB:           fprintf(file, "-"); break;
        case NODE_MUL:           fprintf(file, "*"); break;
        case NODE_DIV:           fprintf(file, "/"); break;
        case NODE_EQ:            fprintf(file, "=="); break;
        case NODE_NE:            fprintf(file, "!="); break;
        case NODE_GT:            fprintf(file, ">"); break;
        case NODE_LT:            fprintf(file, "<"); break;
        case NODE_IF:            fprintf(file, "if"); break;
        case NODE_WHILE:         fprintf(file, "while"); break;
        case NODE_RETURN:        fprintf(file, "ret"); break;

        // Ключевые слова из твоего языка
        case NODE_FUNC_DECL:
            // Имя функции как есть
            fprintf(file, "%s", node->data.string_value);
            break;

        case NODE_VAR_DECL:
            // Для VAR_DECL печатаем только имя переменной
            // Тип не печатаем, как в стандарте
            fprintf(file, "%s", node->data.string_value);
            break;

        // Специальные команды из стандарта
        // Нужно добавить в enum NodeType если их нет:
        // case NODE_IN:  fprintf(file, "in"); break;
        // case NODE_OUT: fprintf(file, "out"); break;
        // case NODE_HLT: fprintf(file, "hlt"); break;

        default:
            fprintf(file, "unknown_%d", node->type);
            break;
    }

    // Для листьев закрываем сразу
    if (node->type == NODE_NUMBER || node->type == NODE_VARIABLE) {
        fprintf(file, " nil nil )");
        return;
    }

    // Переходим на новую строку для детей
    if (node->left || node->right) {
        fprintf(file, "\n");
        DumpNewFormat(node->left, file, depth + 1);
        fprintf(file, "\n");
        DumpNewFormat(node->right, file, depth + 1);

        // Закрывающая скобка с отступом
        fprintf(file, "\n");
        for (int i = 0; i < depth; i++) fprintf(file, "  ");
        fprintf(file, ")");
    } else {
        // Нет детей - закрываем в той же строке
        fprintf(file, " nil nil )");
    }
}

void DumpAST (Node* node, FILE* file)
{
    if (!node || !file) return;
    DumpNewFormat(node, file, 0);
    fprintf(file, "\n");
}
