#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

#define N 1000000

int main() {

    double *A = malloc(N * sizeof(double));
    if (A == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    double sum = 0.0;
    double mean = 0.0;
    double stddev = 0.0;
    double max;

    // Initialization
    srand(0);
    for (int i = 0; i < N; i++)
        A[i] = (double)rand() / RAND_MAX;

    double start_time = omp_get_wtime();

    // First parallel sections: Compute sum and max in parallel
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            // Section 1: Compute sum
            double local_sum = 0.0;
            for (int i = 0; i < N; i++) {
                local_sum += A[i];
            }
            sum = local_sum;
            printf("Section 1 (sum) executed by thread %d\n", omp_get_thread_num());
        }

        #pragma omp section
        {
            // Section 2: Compute max
            double local_max = A[0];
            for (int i = 1; i < N; i++) {
                if (A[i] > local_max)
                    local_max = A[i];
            }
            max = local_max;
            printf("Section 2 (max) executed by thread %d\n", omp_get_thread_num());
        }
    }
    // Implicit barrier here - both sections complete before continuing

    mean = sum / N;

   
    // Compute standard deviation using parallel reduction
    // Putting the std in a sections block with only ONE section is pointless (No parallelism because need to wait other sections to finish)
    stddev = 0.0;
    #pragma omp parallel for reduction(+:stddev)
    for (int i = 0; i < N; i++)
        stddev += (A[i] - mean) * (A[i] - mean);
    stddev = sqrt(stddev / N);
    printf("Section 3 (stddev) computed with parallel for + reduction\n");

    double end_time = omp_get_wtime();

    // Print results
    printf("\n--- Results ---\n");
    printf("Sum     = %f\n", sum);
    printf("Max     = %f\n", max);
    printf("Std Dev = %f\n", stddev);
    printf("Execution time: %f seconds\n", end_time - start_time);

    free(A);
    return 0;
}
