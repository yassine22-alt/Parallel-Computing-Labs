#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void print_matrix(double *mat, int rows, int cols, const char *name) {
    printf("\nMatrix %s (%dx%d):\n", name, rows, cols);
    for (int i = 0; i < rows && i < 5; i++) {  // Print first 5 rows max
        for (int j = 0; j < cols && j < 5; j++) {  // Print first 5 cols max
            printf("%8.2f ", mat[i * cols + j]);
        }
        if (cols > 5) printf("...");
        printf("\n");
    }
    if (rows > 5) printf("...\n");
}

int main(int argc, char *argv[]) {
    int m = 1000;  // Default matrix size
    int n = 1000;
    
    if (argc > 1) {
        m = atoi(argv[1]);
        n = atoi(argv[2] ? argv[2] : argv[1]);
    }
    
    printf("Matrix Multiplication: A(%dx%d) × B(%dx%d) = C(%dx%d)\n", m, n, n, m, m, m);
    
    // Allocate memory dynamically
    double *a = (double *)malloc(m * n * sizeof(double));
    double *b = (double *)malloc(n * m * sizeof(double));
    double *c = (double *)malloc(m * m * sizeof(double));
    
    if (!a || !b || !c) {
        printf("Memory allocation failed!\n");
        return 1;
    }
    
    // Initialize matrices
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            a[i * n + j] = (i + 1) + (j + 1);
        }
    }
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            b[i * m + j] = (i + 1) - (j + 1);
        }
    }
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            c[i * m + j] = 0;
        }
    }
    
    // Matrix multiplication with OpenMP
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < n; k++) {
                c[i * m + j] += a[i * n + k] * b[k * m + j];
            }
        }
    }
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    // Print results
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp master
        num_threads = omp_get_num_threads();
    }
    
    printf("Threads: %d\n", num_threads);
    printf("Execution time: %.6f seconds\n", elapsed);
    printf("GFLOPS: %.3f\n", (2.0 * m * m * n) / (elapsed * 1e9));
    
    // Print small sample of result
    if (m <= 10) {
        print_matrix(c, m, m, "C");
    }
    
    // Free memory
    free(a);
    free(b);
    free(c);
    
    return 0;
}
