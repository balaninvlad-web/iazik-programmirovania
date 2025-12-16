#ifndef TREE_BASE_H
#define TREE_BASE_H

#include <stddef.h>
#include <stdbool.h>

enum  NodeType
{
    NODE_EMPTY,         // Пустой узел
    NODE_SEQUENCE,      // Последовательность (;)

    // Литералы
    NODE_NUMBER,        // Число: 10
    NODE_VARIABLE,      // Переменная: Хохлов

    // Операции
    NODE_ADD,           // + (Включить_в_состав)
    NODE_SUB,           // - (Исключить_из_состава)
    NODE_MUL,           // * (усилить)
    NODE_DIV,           // / (расформировать_до)
    NODE_EQ,            // == (соответствует)
    NODE_NE,            // != (не соответствует)
    NODE_GT,            // > (превосходит_норму)
    NODE_LT,            // < (не превосходит_норму)
    NODE_ASSIGNMENT,    // = (Назначить)

    // Объявления
    NODE_VAR_DECL,      // Объявление переменной: старшина Хохлов
    NODE_FUNC_DECL,     // Объявление функции
    NODE_FUNC_CALL,     // Вызов функции

    // Управление
    NODE_IF,            // if
    NODE_WHILE,         // while
    NODE_RETURN,        // return (вольно)

    // Типы
    NODE_TYPE_INT,      // старшина
    NODE_TYPE_CHAR,     // рядовой
    NODE_TYPE_DOUBLE    // прапорщик
};

struct NodeData
{
    double number_value;
    char* string_value;
    NodeType type_value;
};

struct Node
{
    NodeType type;          // Тип узла
    NodeData data;          // Данные
    struct Node* left;      // Левый потомок
    struct Node* right;     // Правый потомок
    int priority;           // Приоритет операции
};

Node* CreateNumber (double value);
Node* CreateVariable (const char* name);
Node* CreateOperation (NodeType op_type, Node* left, Node* right);
Node* CreateAssignment (Node* variable, Node* value);
Node* CreateVarDeclaration (NodeType var_type, const char* name, Node* init_value);
Node* CreateReturn (Node* expr);
Node* CreateIf (Node* condition, Node* body);
Node* CreateEmpty ();
Node* CreateSequence (Node* first, Node* second);
Node* CreateNode (NodeType type, NodeData data, Node* left, Node* right);
void FreeTree (Node* root);
void PrintTree (Node* node, int depth);
Node* CreateFunctionDeclaration (NodeType return_type, const char* name, Node* params, Node* body);
Node* CreateFunctionCall (const char* func_name, Node* arguments);

#endif
