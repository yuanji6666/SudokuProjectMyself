#include "formula.h"
/*
本模块实现sat问题的解决
*/

// DPLL主算法
int dpll(Formula* formula) {
    // 单子句传播
    if (!unit_propagation(formula)) {
        return 0; // 冲突
    }
    
    // 检查是否所有子句都已满足
    int all_satisfied = 1;
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            all_satisfied = 0;
            break;
        }
        current = current->next;
    }
    
    if (all_satisfied) {
        return 1; // 可满足
    }
    
    // 选择分支变量
    int var = choose_branch_variable(formula);
    if (var == 0) {
        return 0; // 没有未赋值的变量
    }
    
    // 保存当前状态
    FormulaState* state = save_formula_state(formula);
    
    // 尝试正文字分支
    if (assign_literal(formula, var) && dpll(formula)) {
        free_formula_state(state);
        return 1;
    }
    
    // 回溯
    restore_formula_state(formula, state);
    free_formula_state(state);
    
    // 尝试负文字分支
    return assign_literal(formula, -var) && dpll(formula);
}

//单子句传播函数
int unit_propagation(Formula* formula) {
    int changed;
    do {
        changed = 0;
        Clause* current = formula->clauses;//指向当前子句
        while (current != NULL) {
            //只遍历未满足的
            if (!current->marked) {
                int unassigned_count = 0;//未赋值文字的数量
                Literal unit_literal = 0;//单子句中的文字
                int satisfied = 0;//子句是否满足的标志变量
                
                for (int i = 0; i < current->length; i++) {
                    Literal lit = current->literals[i];//存储文字
                    int var = abs(lit);//存储变量名
                    int sign = (lit > 0) ? 1 : -1;//存储赋值
                    
                    if (formula->assignment[var] == 0) {
                        unassigned_count++;
                        unit_literal = lit;
                    } else if (formula->assignment[var] == sign) {
                        satisfied = 1;
                        break;
                    }
                }//遍历子句中每一个文字
                
                if (satisfied) {
                    current->marked = 1;
                    changed = 1;
                } else if (unassigned_count == 0) {
                    return 0; // 冲突
                } else if (unassigned_count == 1) {
                    if (!assign_literal(formula, unit_literal)) {
                        return 0; // 冲突
                    }
                    changed = 1;
                    break; // 重新开始传播
                }
            }
            current = current->next;
        }//依次遍历每一个子句
    } while (changed);//每次变化之后从第一个子句重新开始寻找单子句传播
    
    return 1;
}

//单子句中的单个文字赋值函数
int assign_literal(Formula* formula, Literal literal) {
    int var = abs(literal);
    int value = (literal > 0) ? 1 : -1;
    
    // 检查是否与现有赋值冲突
    if (formula->assignment[var] != 0 && formula->assignment[var] != value) {
        return 0; // 冲突
    }
    
    formula->assignment[var] = value;
    formula->activity[var] += 1.0; // 更新VSIDS活动度
    
    return 1;
}

// VSIDS分支策略
int choose_branch_variable(Formula* formula) {
    double max_score = -1;
    int best_var = 0;
    
    for (int i = 1; i <= formula->var_count; i++) {
        if (formula->assignment[i] == 0) { // 未赋值的变元
            if (formula->activity[i] > max_score) {
                max_score = formula->activity[i];
                best_var = i;
            }
        }
    }//找到活动度最大的变元
    
    // 衰减活动度
    for (int i = 1; i <= formula->var_count; i++) {
        formula->activity[i] *= 0.95;
    }
    
    return best_var;
}

//保存状态
FormulaState* save_formula_state(Formula* formula) {
    FormulaState* state = (FormulaState*)malloc(sizeof(FormulaState));
    
    // 保存赋值
    state->assignment = (int*)malloc((formula->var_count + 1) * sizeof(int));
    memcpy(state->assignment, formula->assignment, (formula->var_count + 1) * sizeof(int));
    
    // 保存标记状态
    state->marked_count = 0;
    Clause* current = formula->clauses;
    while (current != NULL) {
        state->marked_count++;
        current = current->next;
    }
    
    state->marked = (int*)malloc(state->marked_count * sizeof(int));
    int i = 0;
    current = formula->clauses;
    while (current != NULL) {
        state->marked[i++] = current->marked;
        current = current->next;
    }
    
    return state;
}

//释放状态
void free_formula_state(FormulaState* state) {
    free(state->assignment);
    free(state->marked);
    free(state);
}

//恢复状态
void restore_formula_state(Formula* formula, FormulaState* state) {
    // 恢复赋值
    memcpy(formula->assignment, state->assignment, (formula->var_count + 1) * sizeof(int));
    
    // 恢复标记状态
    int i = 0;
    Clause* current = formula->clauses;
    while (current != NULL) {
        current->marked = state->marked[i++];
        current = current->next;
    }
}

