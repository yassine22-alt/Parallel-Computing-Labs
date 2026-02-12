#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 10000

void init_matrix(int n, double *A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i*n + j] = (double)(i + j);
        }
    }
}

void print_matrix(int n, double *A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%6.1f ", A[i*n + j]);
        }
        printf("\n");
    }
}

double sum_matrix(int n, double *A) {
    double sum = 0.0;
    for (int i = 0; i < n*n; i++) {
        sum += A[i];
    }
    return sum;
}

int main() {

    double *A;
    double sum_seq, sum_par;
    double start, end;

    A = (double*) malloc(N * N * sizeof(double));
    if (A == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    printf("=== SEQUENTIAL EXECUTION ===\n");
    double time_seq_start = omp_get_wtime();

    /* Initialization */
    init_matrix(N, A);

    /* Printing (can be commented if N is large) */
    /* print_matrix(N, A); */

    /* Sum computation */
    sum_seq = sum_matrix(N, A);

    double time_seq_end = omp_get_wtime();
    double time_seq = time_seq_end - time_seq_start;

    printf("Sum = %lf\n", sum_seq);
    printf("Execution time = %lf seconds\n\n", time_seq);


    printf("=== PARALLEL EXECUTION (with master and single) ===\n");
    double time_par_start = omp_get_wtime();

    sum_par = 0.0;

    #pragma omp parallel shared(A, sum_par)
    {
        // MASTER: Only master thread (thread 0) initializes
        #pragma omp master
        {
            printf("Thread %d (master) initializing matrix...\n", omp_get_thread_num());
            init_matrix(N, A);
        }

        // Explicit barrier: ensure matrix is ready before printing/summing
        #pragma omp barrier

        // SINGLE: One arbitrary thread prints, others wait at implicit barrier
        #pragma omp single
        {
            printf("Thread %d printing matrix (commented out for large N)...\n", omp_get_thread_num());
            /* print_matrix(N, A); */
        }
        // Implicit barrier after single - all threads synchronized

        // ALL THREADS: Compute sum in parallel using reduction
        #pragma omp for reduction(+:sum_par)
        for (int i = 0; i < N*N; i++) {
            sum_par += A[i];
        }
    }

    double time_par_end = omp_get_wtime();
    double time_par = time_par_end - time_par_start;

    printf("Sum = %lf\n", sum_par);
    printf("Execution time = %lf seconds\n", time_par);

    // Speedup calculation
    double speedup = time_seq / time_par;
    
    printf("\n=== COMPARISON ===\n");
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("Speedup: %.2fx\n", speedup);

    free(A);
    return 0;
}
