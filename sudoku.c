#include "formula.h"

int encode_sudoku_var(int i, int j, int k) {
    return (i-1)*81 + (j-1)*9 + k;
}

void decode_sudoku_var(int var, int* i, int* j, int* k) {
    *k = (var-1) % 9 + 1;
    int temp = (var-1) / 9;
    *j = temp % 9 + 1;
    *i = temp / 9 + 1;
}

Formula* sudoku_to_formula(Sudoku* sudoku, int is_percent) {
    Formula* formula = create_formula(9*9*9);
    
    // 生成单元格约束
    for (int i = 1; i <= 9; i++) {
        for (int j = 1; j <= 9; j++) {
            // 每个单元格至少有一个数字
            Clause* at_least_one = create_clause();
            for (int k = 1; k <= 9; k++) {
                add_literal(at_least_one, encode_sudoku_var(i, j, k));
            }
            add_clause(formula, at_least_one);
            
            // 每个单元格至多有一个数字
            for (int k1 = 1; k1 <= 9; k1++) {
                for (int k2 = k1+1; k2 <= 9; k2++) {
                    Clause* at_most_one = create_clause();
                    add_literal(at_most_one, -encode_sudoku_var(i, j, k1));
                    add_literal(at_most_one, -encode_sudoku_var(i, j, k2));
                    add_clause(formula, at_most_one);
                }
            }
        }
    }
    
    // 生成行约束
    for (int i = 1; i <= 9; i++) {
        for (int k = 1; k <= 9; k++) {
            // 每行至少有一个k
            Clause* row_has_k = create_clause();
            for (int j = 1; j <= 9; j++) {
                add_literal(row_has_k, encode_sudoku_var(i, j, k));
            }
            add_clause(formula, row_has_k);
            
            // 每行至多有一个k
            for (int j1 = 1; j1 <= 9; j1++) {
                for (int j2 = j1+1; j2 <= 9; j2++) {
                    Clause* row_at_most_one = create_clause();
                    add_literal(row_at_most_one, -encode_sudoku_var(i, j1, k));
                    add_literal(row_at_most_one, -encode_sudoku_var(i, j2, k));
                    add_clause(formula, row_at_most_one);
                }
            }
        }
    }
    
    // 生成列约束
    for (int j = 1; j <= 9; j++) {
        for (int k = 1; k <= 9; k++) {
            // 每列至少有一个k
            Clause* col_has_k = create_clause();
            for (int i = 1; i <= 9; i++) {
                add_literal(col_has_k, encode_sudoku_var(i, j, k));
            }
            add_clause(formula, col_has_k);
            
            // 每列至多有一个k
            for (int i1 = 1; i1 <= 9; i1++) {
                for (int i2 = i1+1; i2 <= 9; i2++) {
                    Clause* col_at_most_one = create_clause();
                    add_literal(col_at_most_one, -encode_sudoku_var(i1, j, k));
                    add_literal(col_at_most_one, -encode_sudoku_var(i2, j, k));
                    add_clause(formula, col_at_most_one);
                }
            }
        }
    }
    
    // 生成宫约束
    for (int box_i = 0; box_i < 3; box_i++) {
        for (int box_j = 0; box_j < 3; box_j++) {
            for (int k = 1; k <= 9; k++) {
                // 每宫至少有一个k
                Clause* box_has_k = create_clause();
                for (int i = 1; i <= 3; i++) {
                    for (int j = 1; j <= 3; j++) {
                        int row = box_i * 3 + i;
                        int col = box_j * 3 + j;
                        add_literal(box_has_k, encode_sudoku_var(row, col, k));
                    }
                }
                add_clause(formula, box_has_k);
                
                // 每宫至多有一个k
                for (int pos1 = 0; pos1 < 9; pos1++) {
                    for (int pos2 = pos1+1; pos2 < 9; pos2++) {
                        int i1 = box_i * 3 + (pos1 / 3) + 1;
                        int j1 = box_j * 3 + (pos1 % 3) + 1;
                        int i2 = box_i * 3 + (pos2 / 3) + 1;
                        int j2 = box_j * 3 + (pos2 % 3) + 1;
                        
                        Clause* box_at_most_one = create_clause();
                        add_literal(box_at_most_one, -encode_sudoku_var(i1, j1, k));
                        add_literal(box_at_most_one, -encode_sudoku_var(i2, j2, k));
                        add_clause(formula, box_at_most_one);
                    }
                }
            }
        }
    }
    
    // 如果是百分号数独，添加额外约束
    if (is_percent) {
        // 添加撇对角线约束
        int diag1[] = {1,9, 2,8, 3,7, 4,6, 5,5, 6,4, 7,3, 8,2, 9,1};
        int diag2[] = {1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8,8, 9,9};
        
        for (int k = 1; k <= 9; k++) {
            // 对角线1至少有一个k
            Clause* diag1_has_k = create_clause();
            for (int i = 0; i < 9; i++) {
                int row = diag1[2*i];
                int col = diag1[2*i+1];
                add_literal(diag1_has_k, encode_sudoku_var(row, col, k));
            }
            add_clause(formula, diag1_has_k);
            
            // 对角线1至多有一个k
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1+1; i2 < 9; i2++) {
                    int row1 = diag1[2*i1];
                    int col1 = diag1[2*i1+1];
                    int row2 = diag1[2*i2];
                    int col2 = diag1[2*i2+1];
                    
                    Clause* diag1_at_most_one = create_clause();
                    add_literal(diag1_at_most_one, -encode_sudoku_var(row1, col1, k));
                    add_literal(diag1_at_most_one, -encode_sudoku_var(row2, col2, k));
                    add_clause(formula, diag1_at_most_one);
                }
            }
            
            // 对角线2至少有一个k
            Clause* diag2_has_k = create_clause();
            for (int i = 0; i < 9; i++) {
                int row = diag2[2*i];
                int col = diag2[2*i+1];
                add_literal(diag2_has_k, encode_sudoku_var(row, col, k));
            }
            add_clause(formula, diag2_has_k);
            
            // 对角线2至多有一个k
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1+1; i2 < 9; i2++) {
                    int row1 = diag2[2*i1];
                    int col1 = diag2[2*i1+1];
                    int row2 = diag2[2*i2];
                    int col2 = diag2[2*i2+1];
                    
                    Clause* diag2_at_most_one = create_clause();
                    add_literal(diag2_at_most_one, -encode_sudoku_var(row1, col1, k));
                    add_literal(diag2_at_most_one, -encode_sudoku_var(row2, col2, k));
                    add_clause(formula, diag2_at_most_one);
                }
            }
        }
        
        // 添加窗口约束
        int window1[] = {2,2, 2,3, 2,4, 3,2, 3,3, 3,4, 4,2, 4,3, 4,4};
        int window2[] = {6,6, 6,7, 6,8, 7,6, 7,7, 7,8, 8,6, 8,7, 8,8};
        
        for (int k = 1; k <= 9; k++) {
            // 窗口1至少有一个k
            Clause* window1_has_k = create_clause();
            for (int i = 0; i < 9; i++) {
                int row = window1[2*i];
                int col = window1[2*i+1];
                add_literal(window1_has_k, encode_sudoku_var(row, col, k));
            }
            add_clause(formula, window1_has_k);
            
            // 窗口1至多有一个k
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1+1; i2 < 9; i2++) {
                    int row1 = window1[2*i1];
                    int col1 = window1[2*i1+1];
                    int row2 = window1[2*i2];
                    int col2 = window1[2*i2+1];
                    
                    Clause* window1_at_most_one = create_clause();
                    add_literal(window1_at_most_one, -encode_sudoku_var(row1, col1, k));
                    add_literal(window1_at_most_one, -encode_sudoku_var(row2, col2, k));
                    add_clause(formula, window1_at_most_one);
                }
            }
            
            // 窗口2至少有一个k
            Clause* window2_has_k = create_clause();
            for (int i = 0; i < 9; i++) {
                int row = window2[2*i];
                int col = window2[2*i+1];
                add_literal(window2_has_k, encode_sudoku_var(row, col, k));
            }
            add_clause(formula, window2_has_k);
            
            // 窗口2至多有一个k
            for (int i1 = 0; i1 < 9; i1++) {
                for (int i2 = i1+1; i2 < 9; i2++) {
                    int row1 = window2[2*i1];
                    int col1 = window2[2*i1+1];
                    int row2 = window2[2*i2];
                    int col2 = window2[2*i2+1];
                    
                    Clause* window2_at_most_one = create_clause();
                    add_literal(window2_at_most_one, -encode_sudoku_var(row1, col1, k));
                    add_literal(window2_at_most_one, -encode_sudoku_var(row2, col2, k));
                    add_clause(formula, window2_at_most_one);
                }
            }
        }
    }
    
    // 添加已知数字约束
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (sudoku->grid[i][j] != 0) {
                Clause* given = create_clause();
                add_literal(given, encode_sudoku_var(i+1, j+1, sudoku->grid[i][j]));
                add_clause(formula, given);
            }
        }
    }
    
    return formula;
}

void print_sudoku(Sudoku* sudoku) {
    printf("Sudoku:\n");
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0 && i > 0) {
            printf("------+-------+------\n");
        }
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j > 0) {
                printf("| ");
            }
            if (sudoku->grid[i][j] == 0) {
                printf(". ");
            } else {
                printf("%d ", sudoku->grid[i][j]);
            }
        }
        printf("\n");
    }
}

Sudoku* read_sudoku(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    Sudoku* sudoku = (Sudoku*)malloc(sizeof(Sudoku));
    sudoku->given_count = 0;
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            char c;
            fscanf(file, " %c", &c);
            if (c == '.' || c == '0') {
                sudoku->grid[i][j] = 0;
            } else {
                sudoku->grid[i][j] = c - '0';
                sudoku->given_count++;
            }
        }
    }
    
    fclose(file);
    return sudoku;
}