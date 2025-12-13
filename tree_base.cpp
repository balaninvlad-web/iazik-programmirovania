#include "tree_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node* CreateNode (NodeType type, NodeData data, Node* left, Node* right)
{
    Node* node = (Node*) calloc (1, sizeof(Node));
    if (!node) return NULL;

    node->type = type;
    node->left = left;
    node->right = right;

    node->data.number_value = data.number_value;
    node->data.type_value = data.type_value;

    if (data.string_value)
        node->data.string_value = strdup(data.string_value);
    else
        node->data.string_value = NULL;

    switch (type)
    {
        case NODE_ADD:
        case NODE_SUB:
            node->priority = 1;
            break;
        case NODE_MUL:
        case NODE_DIV:
            node->priority = 2;
            break;
        case NODE_EQ:
        case NODE_NE:
        case NODE_GT:
        case NODE_LT:
            node->priority = 3;
            break;
        case NODE_ASSIGNMENT:
            node->priority = 4;
            break;
        default:
            node->priority = 0;
    }

    return node;
}

Node* CreateNumber (double value)
{
    NodeData data = {};
    data.number_value = value;
    return CreateNode (NODE_NUMBER, data, NULL, NULL);
}

Node* CreateVariable (const char* name)
{
    NodeData data = {};
    data.string_value = (char*) name;
    return CreateNode (NODE_VARIABLE, data, NULL, NULL);
}

Node* CreateOperation (NodeType op_type, Node* left, Node* right)
{
    NodeData data = {};
    return CreateNode(op_type, data, left, right);
}

Node* CreateAssignment (Node* variable, Node* value)
{
    return CreateOperation (NODE_ASSIGNMENT, variable, value);
}

Node* CreateVarDeclaration (NodeType var_type, const char* name, Node* init_value)
{
    NodeData data = {};
    data.string_value = (char*) name;
    data.type_value = var_type;
    return CreateNode (NODE_VAR_DECL, data, init_value, NULL);
}

Node* CreateSequence (Node* first, Node* second)
{
    NodeData data = {};
    return CreateNode (NODE_SEQUENCE, data, first, second);
}

Node* CreateIf (Node* condition, Node* body)
{
    NodeData data = {};
    return CreateNode (NODE_IF, data, condition, body);
}

Node* CreateReturn (Node* expr)
{
    NodeData data = {};
    return CreateNode (NODE_RETURN, data, expr, NULL);
}

Node* CreateEmpty()
{
    NodeData data = {};
    return CreateNode(NODE_EMPTY, data, NULL, NULL);
}

void FreeTree (Node* root)
{
    if (!root) return;

    FreeTree(root->left);
    FreeTree(root->right);

    if (root->data.string_value)
        free(root->data.string_value);

    free (root);
}

Node* CreateFunctionDeclaration (NodeType return_type, const char* name, Node* params, Node* body)
{
    NodeData data = {};
    data.string_value = (char*) name;
    data.type_value = return_type;
    return CreateNode (NODE_FUNC_DECL, data, params, body);
}

Node* CreateFunctionCall (const char* func_name, Node* arguments)
{
    NodeData data = {};
    data.string_value = (char*) func_name;
    return CreateNode (NODE_FUNC_CALL, data, arguments, NULL);
}

void PrintTree(Node* node, int depth)
{
    if (!node) return;

    for (int i = 0; i < depth; i++)
        printf ("  ");

    switch (node->type)
    {
        case NODE_NUMBER:
            printf ("NUMBER: %g\n", node->data.number_value);
            break;

        case NODE_VARIABLE:
            printf ("VAR: %s\n", node->data.string_value);
            break;

        case NODE_ADD: printf ("ADD\n"); break;
        case NODE_SUB: printf ("SUB\n"); break;
        case NODE_MUL: printf ("MUL\n"); break;
        case NODE_DIV: printf ("DIV\n"); break;
        case NODE_ASSIGNMENT: printf ("ASSIGN\n"); break;
        case NODE_EQ: printf ("EQ\n"); break;
        case NODE_NE: printf ("NE\n"); break;
        case NODE_GT: printf ("GT\n"); break;
        case NODE_LT: printf ("LT\n"); break;

        case NODE_VAR_DECL:
            printf ("VAR_DECL: %s (type: %d)\n",
                   node->data.string_value, node->data.type_value);
            break;

        case NODE_SEQUENCE: printf ("SEQUENCE\n"); break;
        case NODE_IF: printf ("IF\n"); break;
        case NODE_RETURN: printf ("RETURN\n"); break;
        case NODE_EMPTY: printf ("EMPTY\n"); break;

        default:
            printf ("UNKNOWN: %d\n", node->type);
            break;
    }

    PrintTree (node->left, depth + 1);
    PrintTree (node->right, depth + 1);
}
