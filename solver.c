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



int unit_propagation(Formula* formula) {
    int changed;
    do {
        changed = 0;
        Clause* current = formula->clauses;
        
        while (current != NULL) {
            if (!current->marked) {
                int unassigned_count = 0;
                Literal unit_literal = 0;
                int satisfied = 0;
                
                for (int i = 0; i < current->length; i++) {
                    Literal lit = current->literals[i];
                    int var = abs(lit);
                    int sign = (lit > 0) ? 1 : -1;
                    
                    if (formula->assignment[var] == 0) {
                        unassigned_count++;
                        unit_literal = lit;
                    } else if (formula->assignment[var] == sign) {
                        satisfied = 1;
                        break;
                    }
                }
                
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
        }
    } while (changed);
    
    return 1;
}


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



void free_formula_state(FormulaState* state) {
    free(state->assignment);
    free(state->marked);
    free(state);
}



// 单子句传播
int has_unit_clause(Formula* formula) {
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            int unassigned_count = 0;
            Literal unit_literal = 0;
            
            for (int i = 0; i < current->length; i++) {
                Literal lit = current->literals[i];
                int var = abs(lit);
                int sign = lit > 0 ? 1 : -1;
                
                if (formula->assignment[var] == 0) {
                    unassigned_count++;
                    unit_literal = lit;
                } else if (formula->assignment[var] == sign) {
                    // 子句已满足
                    unassigned_count = 0;
                    break;
                }
            }
            
            if (unassigned_count == 1) {
                return 1;
            }
        }
        current = current->next;
    }
    return 0;
}

Literal find_unit_clause(Formula* formula) {
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            int unassigned_count = 0;
            Literal unit_literal = 0;
            
            for (int i = 0; i < current->length; i++) {
                Literal lit = current->literals[i];
                int var = abs(lit);
                int sign = lit > 0 ? 1 : -1;
                
                if (formula->assignment[var] == 0) {
                    unassigned_count++;
                    unit_literal = lit;
                } else if (formula->assignment[var] == sign) {
                    // 子句已满足
                    unassigned_count = 0;
                    break;
                }
            }
            
            if (unassigned_count == 1) {
                return unit_literal;
            }
        }
        current = current->next;
    }
    return 0;
}

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

void simplify_formula(Formula* formula) {
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            int satisfied = 0;
            
            for (int i = 0; i < current->length; i++) {
                Literal lit = current->literals[i];
                int var = abs(lit);
                int sign = lit > 0 ? 1 : -1;
                
                if (formula->assignment[var] == sign) {
                    satisfied = 1;
                    break;
                }
            }
            
            if (satisfied) {
                current->marked = 1;
            }
        }
        current = current->next;
    }
}

int is_empty_formula(Formula* formula) {
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            return 0;
        }
        current = current->next;
    }
    return 1;
}

int has_empty_clause(Formula* formula) {
    Clause* current = formula->clauses;
    while (current != NULL) {
        if (!current->marked) {
            int all_false = 1;
            
            for (int i = 0; i < current->length; i++) {
                Literal lit = current->literals[i];
                int var = abs(lit);
                int sign = lit > 0 ? 1 : -1;
                
                if (formula->assignment[var] != -sign) {
                    all_false = 0;
                    break;
                }
            }
            
            if (all_false) {
                return 1;
            }
        }
        current = current->next;
    }
    return 0;
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
    }
    
    // 衰减活动度
    for (int i = 1; i <= formula->var_count; i++) {
        formula->activity[i] *= 0.95;
    }
    
    return best_var;
}

