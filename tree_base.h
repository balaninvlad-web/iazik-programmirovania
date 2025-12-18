#ifndef TREE_BASE_H
#define TREE_BASE_H

#include <stddef.h>
#include <stdbool.h>

enum NodeType
{
    NODE_EMPTY,         // Пустой узел
    NODE_SEQUENCE,      // Последовательность (;)
    NODE_NUMBER,        // Число
    NODE_VARIABLE,      // Переменная
    NODE_ADD,           // +
    NODE_SUB,           // -
    NODE_MUL,           // *
    NODE_DIV,           // /
    NODE_EQ,            // ==
    NODE_NE,            // !=
    NODE_GT,            // >
    NODE_LT,            // <
    NODE_ASSIGNMENT,    // =
    NODE_VAR_DECL,      // Объявление переменной
    NODE_FUNC_DECL,     // Объявление функции
    NODE_FUNC_CALL,     // Вызов функции
    NODE_IF,            // if
    NODE_WHILE,         // while
    NODE_RETURN,        // return
    NODE_TYPE_INT,      // int
    NODE_TYPE_CHAR,     // char
    NODE_TYPE_DOUBLE    // double
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
