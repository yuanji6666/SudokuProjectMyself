#include "formula.h"

void save_result(const char* filename, int result, Formula* formula, double time_ms) {
    char res_filename[256];
    strcpy(res_filename, filename);
    char* dot = strrchr(res_filename, '.');
    if (dot) *dot = '\0';
    strcat(res_filename, ".res");
    
    FILE* file = fopen(res_filename, "w");
    if (!file) {
        printf("Error: Cannot create result file %s\n", res_filename);
        return;
    }
    
    fprintf(file, "s %d\n", result);
    if (result == 1) {
        fprintf(file, "v ");
        for (int i = 1; i <= formula->var_count; i++) {
            fprintf(file, "%d ", formula->assignment[i] > 0 ? i : -i);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "t %.2f\n", time_ms);
    
    fclose(file);
    printf("Result saved to %s\n", res_filename);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <mode> [options]\n", argv[0]);
        printf("Modes:\n");
        printf("  -sat <cnf_file>        Solve SAT problem from CNF file\n");
        printf("  -sudoku <sudoku_file>  Solve normal Sudoku\n");
        printf("  -percent <sudoku_file> Solve Percent Sudoku\n");
        return 1;
    }
    
    if (strcmp(argv[1], "-sat") == 0 && argc >= 3) {
        // SAT求解模式
        const char* cnf_file = argv[2];
        printf("Solving SAT problem from %s\n", cnf_file);
        
        Formula* formula = parse_cnf(cnf_file);
        if (!formula) {
            printf("Error: Failed to parse CNF file\n");
            return 1;
        }
        
        clock_t start = clock();
        int result = dpll(formula);
        clock_t end = clock();
        double time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
        
        printf("Result: %s\n", result ? "SATISFIABLE" : "UNSATISFIABLE");
        printf("Time: %.2f ms\n", time_ms);
        
        save_result(cnf_file, result, formula, time_ms);
        destroy_formula(formula);
        
    } else if ((strcmp(argv[1], "-sudoku") == 0 || strcmp(argv[1], "-percent") == 0) && argc >= 3) {
        // 数独求解模式
        const char* sudoku_file = argv[2];
        int is_percent = strcmp(argv[1], "-percent") == 0;
        
        printf("Solving %s from %s\n", is_percent ? "Percent Sudoku" : "Normal Sudoku", sudoku_file);
        
        Sudoku* sudoku = read_sudoku(sudoku_file);
        if (!sudoku) {
            printf("Error: Failed to read Sudoku file\n");
            return 1;
        }
        
        print_sudoku(sudoku);
        
        Formula* formula = sudoku_to_formula(sudoku, is_percent);
        if (!formula) {
            printf("Error: Failed to convert Sudoku to SAT\n");
            free(sudoku);
            return 1;
        }
        
        clock_t start = clock();
        int result = dpll(formula);
        clock_t end = clock();
        double time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
        
        if (result) {
            printf("Sudoku solved successfully!\n");
            // 将SAT解转换为数独解
            for (int i = 1; i <= formula->var_count; i++) {
                if (formula->assignment[i] > 0) {
                    int row, col, num;
                    decode_sudoku_var(i, &row, &col, &num);
                    sudoku->grid[row-1][col-1] = num;
                }
            }
            print_sudoku(sudoku);
        } else {
            printf("No solution found for the Sudoku\n");
        }
        
        printf("Time: %.2f ms\n", time_ms);
        save_result(sudoku_file, result, formula, time_ms);
        
        destroy_formula(formula);
        free(sudoku);
        
    } else {
        printf("Invalid arguments\n");
        return 1;
    }
    
    return 0;
}