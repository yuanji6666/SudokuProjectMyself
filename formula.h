#ifndef FORMULA_H
#define FORMULA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// 文字表示：正数表示正文字，负数表示负文字
typedef int Literal;

// 子句结构
typedef struct Clause {
    Literal* literals;  // 文字数组
    int length;         // 子句长度
    int marked;         // 标记是否已被满足
    struct Clause* next; // 指向下一个子句
} Clause;

// 公式结构（CNF）
typedef struct {
    int var_count;      // 变元数量
    int clause_count;   // 子句数量
    Clause* clauses;    // 子句链表头指针
    int* assignment;    // 变元赋值数组 (0-未赋值, 1-真, -1-假)
    int* decision_level; // 决策级别数组
    double* activity;   // VSIDS活动度数组
    Clause** watch;     // 监视文字表
    int* watch_count;   // 每个变元的监视子句数量
} Formula;

// 数独游戏结构
typedef struct {
    int grid[9][9];     // 数独网格
    int given_count;    // 已知数字数量
} Sudoku;
typedef struct {
    int* assignment;
    int* marked;
    int marked_count;
} FormulaState;



//--------------函数声明--------------


//dpll算法核心
int dpll(Formula* formula);
int unit_propagation(Formula* formula);
int assign_literal(Formula* formula, Literal literal);
int choose_branch_variable(Formula* formula);
//状态存，取，释放
FormulaState* save_formula_state(Formula* formula);
void restore_formula_state(Formula* formula, FormulaState* state);
void free_formula_state(FormulaState* state);
//cnf文件解析函数
Formula* create_formula(int var_count);
void destroy_formula(Formula* formula);
Clause* create_clause();
void add_literal(Clause* clause, Literal literal);
void add_clause(Formula* formula, Clause* clause);
Formula* parse_cnf(const char* filename);
//数独解决函数
int encode_sudoku_var(int i, int j, int k);
void decode_sudoku_var(int var, int* i, int* j, int* k);
Formula* sudoku_to_formula(Sudoku* sudoku, int is_percent);
void print_sudoku(Sudoku* sudoku);
Sudoku* read_sudoku(const char* filename);
//结果保存
void save_result(const char* filename, int result, Formula* formula, double time_ms);

#endif