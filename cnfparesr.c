#include "formula.h"
/*

本模块用来将cnf文件解析到数据结构中

*/

//读取cnf文件
Formula* parse_cnf(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("打开文件“ %s ”失败\n", filename);
        return NULL;
    }
    
    char line[1024];
    int var_count = 0, clause_count = 0;
    Formula* formula = NULL;
    
    // 读取文件头
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') {
            continue; // 跳过注释行
        } else if (line[0] == 'p') {
            // 解析问题行
            sscanf(line, "p cnf %d %d", &var_count, &clause_count);
            formula = create_formula(var_count);
            break;
        }
    }
    
    if (!formula) {
        fclose(file);
        return NULL;
    }
    
    // 读取子句
    Clause* current_clause = create_clause();
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue; // 跳过注释行
        
        char* token = strtok(line, " \t\n");
        while (token != NULL) {
            int value = atoi(token);
            if (value == 0) {
                // 子句结束
                if (current_clause->length > 0) {
                    add_clause(formula, current_clause);
                    current_clause = create_clause();
                }
            } else {
                // 添加文字到当前子句
                add_literal(current_clause, value);
            }
            token = strtok(NULL, " \t\n");
        }
    }
    
    // 处理最后一个子句（如果有）
    if (current_clause->length > 0) {
        add_clause(formula, current_clause);
    } else {
        free(current_clause->literals);
        free(current_clause);
    }
    
    fclose(file);
    return formula;
}

//formula初始化
Formula* create_formula(int var_count) {
    Formula* formula = (Formula*)malloc(sizeof(Formula));
    formula->var_count = var_count;
    formula->clause_count = 0;
    formula->clauses = NULL;
    
    // 分配赋值数组和决策级别数组
    formula->assignment = (int*)calloc(var_count + 1, sizeof(int));
    formula->decision_level = (int*)calloc(var_count + 1, sizeof(int));
    formula->activity = (double*)calloc(var_count + 1, sizeof(double));
    
    // 初始化监视表
    formula->watch = (Clause**)calloc(var_count + 1, sizeof(Clause*));
    formula->watch_count = (int*)calloc(var_count + 1, sizeof(int));
    
    return formula;
}

//删除formula
void destroy_formula(Formula* formula) {
    if (!formula) return;
    
    // 释放所有子句
    Clause* current = formula->clauses;
    while (current != NULL) {
        Clause* next = current->next;
        free(current->literals);
        free(current);
        current = next;
    }
    
    // 释放数组
    free(formula->assignment);
    free(formula->decision_level);
    free(formula->activity);
    free(formula->watch);
    free(formula->watch_count);
    
    free(formula);
}

//初始化子句
Clause* create_clause() {
    Clause* clause = (Clause*)malloc(sizeof(Clause));
    clause->literals = NULL;
    clause->length = 0;
    clause->marked = 0;
    clause->next = NULL;
    return clause;
}

//添加文字
void add_literal(Clause* clause, Literal literal) {
    clause->length++;
    clause->literals = (Literal*)realloc(clause->literals, clause->length * sizeof(Literal));
    clause->literals[clause->length - 1] = literal;
}

//添加子句
void add_clause(Formula* formula, Clause* clause) {
    formula->clause_count++;
    clause->next = formula->clauses;
    formula->clauses = clause;
}