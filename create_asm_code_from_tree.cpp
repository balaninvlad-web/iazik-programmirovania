#include "create_asm_code_from_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

CodeGenContext* CtorCodeGen(FILE* output) {
    CodeGenContext* ctx = (CodeGenContext*)calloc(1, sizeof(CodeGenContext));
    if (!ctx) return NULL;

    ctx->output = output;
    ctx->label_counter = 0;
    ctx->var_counter = 0;
    ctx->temp_counter = 0;
    ctx->var_count = 0;
    ctx->func_count = 0;
    ctx->in_function = 0;
    ctx->current_func = NULL;

    // Начальные размеры таблиц
    ctx->var_capacity = 20;
    ctx->func_capacity = 10;

    ctx->var_table = (VariableInfo*)calloc(ctx->var_capacity, sizeof(VariableInfo));
    ctx->func_table = (FunctionInfo*)calloc(ctx->func_capacity, sizeof(FunctionInfo));

    if (!ctx->var_table || !ctx->func_table) {
        if (ctx->var_table) free(ctx->var_table);
        if (ctx->func_table) free(ctx->func_table);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void DtorCodeGen(CodeGenContext* ctx) {
    if (!ctx) return;

    // Освобождаем таблицу переменных
    for (int i = 0; i < ctx->var_count; i++) {
        free(ctx->var_table[i].name);
    }
    free(ctx->var_table);

    // Освобождаем таблицу функций
    for (int i = 0; i < ctx->func_count; i++) {
        free(ctx->func_table[i].name);
    }
    free(ctx->func_table);

    free(ctx->current_func);

    if (ctx->output && ctx->output != stdout && ctx->output != stderr) {
        fclose(ctx->output);
    }

    free(ctx);
}

int NewLabel(CodeGenContext* ctx) {
    return ctx->label_counter++;
}

int AddVariable(CodeGenContext* ctx, const char* var_name, int is_local) {
    // Проверяем, есть ли уже такая переменная
    for (int i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->var_table[i].name, var_name) == 0 &&
            ctx->var_table[i].is_local == is_local) {
            return ctx->var_table[i].address;
        }
    }

    // Увеличиваем таблицу при необходимости
    if (ctx->var_count >= ctx->var_capacity) {
        ctx->var_capacity *= 2;
        VariableInfo* new_table = (VariableInfo*)realloc(ctx->var_table,
                                                        ctx->var_capacity * sizeof(VariableInfo));
        if (!new_table) return -1;
        ctx->var_table = new_table;
    }

    // Добавляем новую переменную
    ctx->var_table[ctx->var_count].name = strdup(var_name);
    ctx->var_table[ctx->var_count].is_local = is_local;

    // Назначаем адрес
    if (is_local) {
        // Локальные переменные размещаются на стеке (отрицательные смещения)
        ctx->var_table[ctx->var_count].address = -(ctx->var_count + 1) * 4;
    } else {
        // Глобальные переменные размещаются в фиксированных адресах
        ctx->var_table[ctx->var_count].address = 1000 + ctx->var_counter * 4;
        ctx->var_counter++;
    }

    int address = ctx->var_table[ctx->var_count].address;
    ctx->var_count++;

    return address;
}

int AddFunction(CodeGenContext* ctx, const char* func_name) {
    // Проверяем, есть ли уже такая функция
    for (int i = 0; i < ctx->func_count; i++) {
        if (strcmp(ctx->func_table[i].name, func_name) == 0) {
            return ctx->func_table[i].start_label;
        }
    }

    // Увеличиваем таблицу при необходимости
    if (ctx->func_count >= ctx->func_capacity) {
        ctx->func_capacity *= 2;
        FunctionInfo* new_table = (FunctionInfo*)realloc(ctx->func_table,
                                                        ctx->func_capacity * sizeof(FunctionInfo));
        if (!new_table) return -1;
        ctx->func_table = new_table;
    }

    // Добавляем новую функцию
    ctx->func_table[ctx->func_count].name = strdup(func_name);
    ctx->func_table[ctx->func_count].start_label = NewLabel(ctx);
    ctx->func_table[ctx->func_count].local_var_count = 0;

    int label = ctx->func_table[ctx->func_count].start_label;
    ctx->func_count++;

    return label;
}

int GetVarAddress(CodeGenContext* ctx, const char* var_name) {
    // Сначала ищем локальную переменную (если внутри функции)
    if (ctx->in_function) {
        for (int i = 0; i < ctx->var_count; i++) {
            if (strcmp(ctx->var_table[i].name, var_name) == 0 &&
                ctx->var_table[i].is_local) {
                return ctx->var_table[i].address;
            }
        }
    }

    // Затем ищем глобальную переменную
    for (int i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->var_table[i].name, var_name) == 0 &&
            !ctx->var_table[i].is_local) {
            return ctx->var_table[i].address;
        }
    }

    // Если не нашли, создаём глобальную переменную
    return AddVariable(ctx, var_name, 0);
}

void EnterFunction(CodeGenContext* ctx, const char* func_name) {
    free(ctx->current_func);
    ctx->current_func = strdup(func_name);
    ctx->in_function = 1;
}

void ExitFunction(CodeGenContext* ctx) {
    free(ctx->current_func);
    ctx->current_func = NULL;
    ctx->in_function = 0;
}

void GenExpression(CodeGenContext* ctx, Node* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_NUMBER:
            fprintf(ctx->output, "PUSH %g\n", node->data.number_value);
            break;

        case NODE_VARIABLE: {
            int addr = GetVarAddress(ctx, node->data.string_value);
            // Загружаем переменную из памяти
            fprintf(ctx->output, "PUSH %d\n", addr);
            fprintf(ctx->output, "POPR RAX\n");
            fprintf(ctx->output, "PUSHM RAX\n");
            break;
        }

        case NODE_ADD:
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "ADD\n");
            break;

        case NODE_SUB:
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "SUB\n");
            break;

        case NODE_MUL:
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "MUL\n");
            break;

        case NODE_DIV:
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "DIV\n");
            break;

        case NODE_EQ: {
            // Генерируем сравнение: left == right
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "SUB\n");  // left - right

            // Создаём метки для перехода
            int true_label = NewLabel(ctx);
            int end_label = NewLabel(ctx);

            // Если результат == 0, то равны
            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JE :label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 0\n");  // false
            fprintf(ctx->output, "JMP :label_%d\n", end_label);
            fprintf(ctx->output, ":label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 1\n");  // true
            fprintf(ctx->output, ":label_%d\n", end_label);
            break;
        }

        case NODE_NE: {
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "SUB\n");

            int true_label = NewLabel(ctx);
            int end_label = NewLabel(ctx);

            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JNE :label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JMP :label_%d\n", end_label);
            fprintf(ctx->output, ":label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 1\n");
            fprintf(ctx->output, ":label_%d\n", end_label);
            break;
        }

        case NODE_GT: {
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "SUB\n");

            int true_label = NewLabel(ctx);
            int end_label = NewLabel(ctx);

            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JA :label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JMP :label_%d\n", end_label);
            fprintf(ctx->output, ":label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 1\n");
            fprintf(ctx->output, ":label_%d\n", end_label);
            break;
        }

        case NODE_LT: {
            GenExpression(ctx, node->left);
            GenExpression(ctx, node->right);
            fprintf(ctx->output, "SUB\n");

            int true_label = NewLabel(ctx);
            int end_label = NewLabel(ctx);

            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JB :label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 0\n");
            fprintf(ctx->output, "JMP :label_%d\n", end_label);
            fprintf(ctx->output, ":label_%d\n", true_label);
            fprintf(ctx->output, "PUSH 1\n");
            fprintf(ctx->output, ":label_%d\n", end_label);
            break;
        }

        case NODE_FUNC_CALL:
            GenFuncCall(ctx, node);
            break;

        default:
            fprintf(ctx->output, "; Неподдерживаемое выражение типа: %d\n", node->type);
            break;
    }
}

void GenAssignment(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_ASSIGNMENT) return;

    // Генерируем правое выражение
    GenExpression(ctx, node->right);

    // Получаем адрес переменной
    if (node->left && node->left->type == NODE_VARIABLE) {
        int addr = GetVarAddress(ctx, node->left->data.string_value);

        // Сохраняем значение в переменную
        fprintf(ctx->output, "POPR RBX\n");  // Значение в RBX
        fprintf(ctx->output, "PUSH %d\n", addr);
        fprintf(ctx->output, "POPR RAX\n");  // Адрес в RAX
        fprintf(ctx->output, "POPM RAX\n");  // Сохраняем значение
    }
}

void GenSequence(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_SEQUENCE) return;

    GenerateCode(ctx, node->left);
    GenerateCode(ctx, node->right);
}

void GenIf(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_IF) return;

    int false_label = NewLabel(ctx);
    int end_label = NewLabel(ctx);

    // Генерируем условие (оставляет 0 или 1 на стеке)
    GenExpression(ctx, node->left);

    // Если условие ложно (0), переходим к false_label
    fprintf(ctx->output, "PUSH 0\n");
    fprintf(ctx->output, "JE :label_%d\n", false_label);

    // Тело if
    GenerateCode(ctx, node->right);

    // Переход к концу
    fprintf(ctx->output, "JMP :label_%d\n", end_label);

    // Метка для ложного условия
    fprintf(ctx->output, ":label_%d\n", false_label);

    // Метка конца if
    fprintf(ctx->output, ":label_%d\n", end_label);
}

void GenWhile(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_WHILE) return;

    int start_label = NewLabel(ctx);
    int end_label = NewLabel(ctx);

    // Метка начала цикла
    fprintf(ctx->output, ":label_%d\n", start_label);

    // Генерируем условие
    GenExpression(ctx, node->left);

    // Если условие ложно, выходим из цикла
    fprintf(ctx->output, "PUSH 0\n");
    fprintf(ctx->output, "JE :label_%d\n", end_label);

    // Тело цикла
    GenerateCode(ctx, node->right);

    // Переход к проверке условия
    fprintf(ctx->output, "JMP :label_%d\n", start_label);

    // Метка конца цикла
    fprintf(ctx->output, ":label_%d\n", end_label);
}

void GenVarDecl(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_VAR_DECL) return;

    // Определяем, локальная или глобальная переменная
    int is_local = ctx->in_function ? 1 : 0;
    int addr = AddVariable(ctx, node->data.string_value, is_local);

    // Если есть инициализатор
    if (node->left) {
        GenExpression(ctx, node->left);

        // Сохраняем значение в переменную
        fprintf(ctx->output, "POPR RBX\n");  // Значение в RBX
        fprintf(ctx->output, "PUSH %d\n", addr);
        fprintf(ctx->output, "POPR RAX\n");  // Адрес в RAX
        fprintf(ctx->output, "POPM RAX\n");  // Сохраняем
    }
}

void GenFuncDecl(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_FUNC_DECL) return;

    char* func_name = node->data.string_value;

    // Регистрируем функцию
    int func_label = AddFunction(ctx, func_name);

    // Генерируем метку функции
    fprintf(ctx->output, "\n; === Функция %s ===\n", func_name);
    fprintf(ctx->output, ":func_%d\n", func_label);

    // Входим в контекст функции
    EnterFunction(ctx, func_name);

    // Генерируем пролог (если нужно)
    fprintf(ctx->output, "; Пролог функции\n");

    // Генерируем параметры (левое поддерево)
    if (node->left) {
        fprintf(ctx->output, "; Параметры:\n");
        // TODO: обработать параметры
    }

    // Генерируем тело функции (правое поддерево)
    if (node->right) {
        GenerateCode(ctx, node->right);
    }

    // Если функция не заканчивается return, добавляем RET
    fprintf(ctx->output, "RET\n");

    // Выходим из контекста функции
    ExitFunction(ctx);
}

void GenFuncCall(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_FUNC_CALL) return;

    char* func_name = node->data.string_value;

    // Ищем функцию в таблице
    int func_label = -1;
    for (int i = 0; i < ctx->func_count; i++) {
        if (strcmp(ctx->func_table[i].name, func_name) == 0) {
            func_label = ctx->func_table[i].start_label;
            break;
        }
    }

    if (func_label >= 0) {
        fprintf(ctx->output, "CALL :func_%d\n", func_label);
    } else {
        fprintf(ctx->output, "; Ошибка: функция '%s' не определена\n", func_name);
    }
}

void GenReturn(CodeGenContext* ctx, Node* node) {
    if (!node || node->type != NODE_RETURN) return;

    // Генерируем возвращаемое выражение
    if (node->left) {
        GenExpression(ctx, node->left);
    } else {
        fprintf(ctx->output, "PUSH 0\n");
    }

    // Выводим значение для отладки
    fprintf(ctx->output, "OUT\n");

    // Возврат из функции
    fprintf(ctx->output, "RET\n");
}

void GenerateCode(CodeGenContext* ctx, Node* node) {
    if (!node || !ctx || !ctx->output) return;

    switch (node->type) {
        case NODE_NUMBER:
        case NODE_VARIABLE:
        case NODE_ADD:
        case NODE_SUB:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_EQ:
        case NODE_NE:
        case NODE_GT:
        case NODE_LT:
        case NODE_FUNC_CALL:
            GenExpression(ctx, node);
            break;

        case NODE_ASSIGNMENT:
            GenAssignment(ctx, node);
            break;

        case NODE_SEQUENCE:
            GenSequence(ctx, node);
            break;

        case NODE_IF:
            GenIf(ctx, node);
            break;

        case NODE_WHILE:
            GenWhile(ctx, node);
            break;

        case NODE_VAR_DECL:
            GenVarDecl(ctx, node);
            break;

        case NODE_FUNC_DECL:
            GenFuncDecl(ctx, node);
            break;

        case NODE_RETURN:
            GenReturn(ctx, node);
            break;

        default:
            fprintf(ctx->output, "; Неподдерживаемый узел: %d\n", node->type);
            break;
    }
}
