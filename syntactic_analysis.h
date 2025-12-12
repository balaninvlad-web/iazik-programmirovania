#ifndef TREE_BASE_H
#define TREE_BASE_H

#include <stddef.h>

// Типы узлов AST
typedef enum {
    NODE_EMPTY,
    NODE_SEQUENCE,

    NODE_VAR_DECLARATION,  // Объявление переменной
    NODE_FUNCTION,         // Объявление функции
    NODE_PARAMETER,        // Параметр функции

    NODE_TYPE_INT,         // старшина
    NODE_TYPE_CHAR,        // рядовой
    NODE_TYPE_DOUBLE,      // прапорщик

    NODE_ASSIGNMENT,       // Назначить
    NODE_IF,               // Приготовиться_к_исполнению_по_получении_приказа
    NODE_RETURN,           // вольно

    NODE_ADD,              // Включить_в_состав
    NODE_SUB,              // Исключить_из_состава
    NODE_MUL,              // усилить
    NODE_DIV,              // расформировать_до

    // Операторы сравнения
    NODE_EQ,               // соответствует
    NODE_NE,               // не соответствует
    NODE_GT,               // превосходит_норму
    NODE_LT,               // не превосходит_норму

    // Литералы
    NODE_NUMBER,
    NODE_VARIABLE,         // Идентификатор (переменная)
    NODE_FUNCTION_CALL,

    // Символьные токены
    NODE_SEMICOLON,
    NODE_LPAREN,
    NODE_RPAREN,
    NODE_LBRACE,
    NODE_RBRACE,
    NODE_COMMA
} NodeType;

// Типы операций
typedef enum {
    OP_ADD,      // Включить_в_состав
    OP_SUB,      // Исключить_из_состава
    OP_MUL,      // усилить
    OP_DIV,      // расформировать_до
    OP_EQ,       // соответствует
    OP_NE,       // не соответствует
    OP_GT,       // превосходит_норму
    OP_LT,       // не превосходит_норму
    OP_ASSIGN    // Назначить
} OperationType;

// Определение переменной
typedef struct {
    char* name;
    NodeType var_type;  // NODE_TYPE_INT, NODE_TYPE_CHAR, NODE_TYPE_DOUBLE
} VariableDefinition;

// Данные вызова функции
typedef struct {
    char* func_name;
    struct Node* args;  // Список аргументов (NODE_SEQUENCE)
} FuncCallData;

// Значение узла дерева
typedef union {
    double             num_value;         // для чисел
    OperationType      op_value;          // для операций
    VariableDefinition var_definition;    // для определения переменной
    char*              func_name;         // для имен функций
    FuncCallData       func_call;         // для вызовов функций
    char*              identifier;        // для идентификаторов
} ValueOfTreeElement;

// Узел дерева
typedef struct Node {
    ValueOfTreeElement  data;
    NodeType            type;
    struct Node*        left;
    struct Node*        right;
    int                 priority;  // Приоритет операции (0 для чисел и переменных)
} Node;

// Функции создания узлов
Node* CreateNode(NodeType type, ValueOfTreeElement data, Node* left, Node* right);
Node* CreateEmptyNode(void);
Node* CreateNumberNode(double value);
Node* CreateVariableNode(const char* name);
Node* CreateBinaryOpNode(OperationType op, Node* left, Node* right);
Node* CreateAssignNode(Node* variable, Node* value);
Node* CreateVarDeclNode(NodeType var_type, const char* name, Node* init_value);
Node* CreateSequenceNode(Node* first, Node* second);
Node* CreateIfNode(Node* condition, Node* body);
Node* CreateReturnNode(Node* expr);
Node* CreateFunctionNode(const char* name, Node* params, Node* body);
Node* CreateFunctionCallNode(const char* name, Node* args);

// Вспомогательные функции
void FreeSubtree(Node* node);
const char* NodeTypeToString(NodeType type);
void PrintTree(Node* node, int depth);

#endif
