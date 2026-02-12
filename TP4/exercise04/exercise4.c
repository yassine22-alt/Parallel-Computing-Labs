#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void dmvm(int n, int m,
          double *lhs, double *rhs, double *mat)
{
    for (int c = 0; c < n; ++c) {
        int offset = m * c;
        for (int r = 0; r < m; ++r)
            lhs[r] += mat[r + offset] * rhs[c];
    }
}

// Version 1: Parallel with implicit barrier
void dmvm_v1_implicit_barrier(int n, int m,
                               double *lhs, double *rhs, double *mat)
{
    #pragma omp parallel for schedule(static)
    for (int c = 0; c < n; ++c) {
        int offset = m * c;
        for (int r = 0; r < m; ++r)
            lhs[r] += mat[r + offset] * rhs[c];
    }
    // Implicit barrier here - all threads wait
}

// Version 2: Dynamic scheduling with nowait
void dmvm_v2_dynamic_nowait(int n, int m,
                             double *lhs, double *rhs, double *mat)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic) nowait
        for (int c = 0; c < n; ++c) {
            int offset = m * c;
            for (int r = 0; r < m; ++r)
                lhs[r] += mat[r + offset] * rhs[c];
        }
        // No implicit barrier - threads continue immediately
    }
}

// Version 3: Static scheduling with nowait
void dmvm_v3_static_nowait(int n, int m,
                            double *lhs, double *rhs, double *mat)
{
    #pragma omp parallel
    {
        #pragma omp for schedule(static) nowait
        for (int c = 0; c < n; ++c) {
            int offset = m * c;
            for (int r = 0; r < m; ++r)
                lhs[r] += mat[r + offset] * rhs[c];
        }
        // No implicit barrier - threads continue immediately
    }
}

double calculate_mflops(int n, int m, double time_sec) {
    // FLOPs: n iterations * m rows * 2 ops (multiply + add)
    long long flops = (long long)n * m * 2;
    return (flops / time_sec) / 1e6;
}

void run_benchmark(int num_threads) {
    const int n = 40000; // columns
    const int m = 6000;   // rows
    
    printf("\n========================================\n");
    printf("Running with %d thread(s)\n", num_threads);
    printf("========================================\n");
    
    omp_set_num_threads(num_threads);
    
    // Allocate memory
    double *mat = malloc(n * m * sizeof(double));
    double *rhs = malloc(n * sizeof(double));
    double *lhs1 = malloc(m * sizeof(double));
    double *lhs2 = malloc(m * sizeof(double));
    double *lhs3 = malloc(m * sizeof(double));
    
    // Initialization
    for (int c = 0; c < n; ++c) {
        rhs[c] = 1.0;
        for (int r = 0; r < m; ++r)
            mat[r + c*m] = 1.0;
    }
    
    double time_seq = 0.0;
    
    // Sequential version (only for thread count = 1 to get baseline)
    if (num_threads == 1) {
        for (int r = 0; r < m; ++r)
            lhs1[r] = 0.0;
        
        double start = omp_get_wtime();
        dmvm(n, m, lhs1, rhs, mat);
        double end = omp_get_wtime();
        time_seq = end - start;
        
        printf("\nSequential (baseline):\n");
        printf("  Time:    %.4f s\n", time_seq);
        printf("  MFLOP/s: %.2f\n", calculate_mflops(n, m, time_seq));
    }
    
    // Version 1: Implicit barrier
    for (int r = 0; r < m; ++r)
        lhs1[r] = 0.0;
    
    double start1 = omp_get_wtime();
    dmvm_v1_implicit_barrier(n, m, lhs1, rhs, mat);
    double end1 = omp_get_wtime();
    double time_v1 = end1 - start1;
    
    // Version 2: Dynamic + nowait
    for (int r = 0; r < m; ++r)
        lhs2[r] = 0.0;
    
    double start2 = omp_get_wtime();
    dmvm_v2_dynamic_nowait(n, m, lhs2, rhs, mat);
    double end2 = omp_get_wtime();
    double time_v2 = end2 - start2;
    
    // Version 3: Static + nowait
    for (int r = 0; r < m; ++r)
        lhs3[r] = 0.0;
    
    double start3 = omp_get_wtime();
    dmvm_v3_static_nowait(n, m, lhs3, rhs, mat);
    double end3 = omp_get_wtime();
    double time_v3 = end3 - start3;
    
    // Calculate metrics
    double speedup_v1 = (num_threads == 1) ? 1.0 : time_seq / time_v1;
    double speedup_v2 = (num_threads == 1) ? 1.0 : time_seq / time_v2;
    double speedup_v3 = (num_threads == 1) ? 1.0 : time_seq / time_v3;
    
    double efficiency_v1 = speedup_v1 / num_threads * 100;
    double efficiency_v2 = speedup_v2 / num_threads * 100;
    double efficiency_v3 = speedup_v3 / num_threads * 100;
    
    printf("\nVersion 1 (implicit barrier):\n");
    printf("  Time:       %.4f s\n", time_v1);
    printf("  Speedup:    %.2fx\n", speedup_v1);
    printf("  Efficiency: %.1f%%\n", efficiency_v1);
    printf("  MFLOP/s:    %.2f\n", calculate_mflops(n, m, time_v1));
    
    printf("\nVersion 2 (dynamic + nowait):\n");
    printf("  Time:       %.4f s\n", time_v2);
    printf("  Speedup:    %.2fx\n", speedup_v2);
    printf("  Efficiency: %.1f%%\n", efficiency_v2);
    printf("  MFLOP/s:    %.2f\n", calculate_mflops(n, m, time_v2));
    
    printf("\nVersion 3 (static + nowait):\n");
    printf("  Time:       %.4f s\n", time_v3);
    printf("  Speedup:    %.2fx\n", speedup_v3);
    printf("  Efficiency: %.1f%%\n", efficiency_v3);
    printf("  MFLOP/s:    %.2f\n", calculate_mflops(n, m, time_v3));
    
    printf("\nComparison:\n");
    printf("  V1 vs V3: %.2fx %s\n", 
           time_v1/time_v3, 
           time_v1 > time_v3 ? "(V3 faster - nowait helps!)" : "(barrier not costly here)");
    printf("  V2 vs V3: %.2fx %s\n",
           time_v2/time_v3,
           time_v2 > time_v3 ? "(static better - less overhead)" : "(dynamic better - load balance)");
    
    // Verification (check first few results match)
    printf("\nVerification (first 3 results):\n");
    for (int r = 0; r < 3; ++r) {
        printf("  lhs[%d]: V1=%.1f, V2=%.1f, V3=%.1f %s\n", 
               r, lhs1[r], lhs2[r], lhs3[r],
               (lhs1[r] == lhs2[r] && lhs2[r] == lhs3[r]) ? "✓" : "✗ MISMATCH!");
    }
    
    free(mat);
    free(rhs);
    free(lhs1);
    free(lhs2);
    free(lhs3);
}

int main(void)
{
    printf("Matrix-Vector Multiplication Benchmark\n");
    printf("Matrix: 40000 x 600\n");
    printf("FLOPs per run: %lld\n", (long long)40000 * 600 * 2);
    
    // Get sequential baseline first
    printf("\n=== GETTING SEQUENTIAL BASELINE ===\n");
    run_benchmark(1);
    
    // Store sequential time for later speedup calculations
    const int n = 40000;
    const int m = 600;
    double *mat = malloc(n * m * sizeof(double));
    double *rhs = malloc(n * sizeof(double));
    double *lhs = malloc(m * sizeof(double));
    
    for (int c = 0; c < n; ++c) {
        rhs[c] = 1.0;
        for (int r = 0; r < m; ++r)
            mat[r + c*m] = 1.0;
    }
    for (int r = 0; r < m; ++r)
        lhs[r] = 0.0;
    
    double start_seq = omp_get_wtime();
    dmvm(n, m, lhs, rhs, mat);
    double end_seq = omp_get_wtime();
    double time_sequential = end_seq - start_seq;
    
    free(mat);
    free(rhs);
    free(lhs);
    
    printf("\n=== PARALLEL SCALABILITY TEST ===\n");
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tests = 5;
    
    // Store results for plotting data
    printf("\n=== RESULTS FOR PLOTTING ===\n");
    printf("Threads\tV1_Time\tV1_Speedup\tV1_Efficiency\tV1_MFLOPS\tV3_Time\tV3_Speedup\tV3_Efficiency\tV3_MFLOPS\n");
    
    for (int i = 0; i < num_tests; i++) {
        int threads = thread_counts[i];
        
        omp_set_num_threads(threads);
        
        // Allocate
        mat = malloc(n * m * sizeof(double));
        rhs = malloc(n * sizeof(double));
        double *lhs1 = malloc(m * sizeof(double));
        double *lhs3 = malloc(m * sizeof(double));
        
        // Initialize
        for (int c = 0; c < n; ++c) {
            rhs[c] = 1.0;
            for (int r = 0; r < m; ++r)
                mat[r + c*m] = 1.0;
        }
        
        // V1
        for (int r = 0; r < m; ++r)
            lhs1[r] = 0.0;
        double start1 = omp_get_wtime();
        dmvm_v1_implicit_barrier(n, m, lhs1, rhs, mat);
        double end1 = omp_get_wtime();
        double time_v1 = end1 - start1;
        
        // V3
        for (int r = 0; r < m; ++r)
            lhs3[r] = 0.0;
        double start3 = omp_get_wtime();
        dmvm_v3_static_nowait(n, m, lhs3, rhs, mat);
        double end3 = omp_get_wtime();
        double time_v3 = end3 - start3;
        
        double speedup_v1 = time_sequential / time_v1;
        double speedup_v3 = time_sequential / time_v3;
        double eff_v1 = speedup_v1 / threads * 100;
        double eff_v3 = speedup_v3 / threads * 100;
        double mflops_v1 = calculate_mflops(n, m, time_v1);
        double mflops_v3 = calculate_mflops(n, m, time_v3);
        
        printf("%d\t%.4f\t%.2f\t%.1f\t%.2f\t%.4f\t%.2f\t%.1f\t%.2f\n",
               threads, time_v1, speedup_v1, eff_v1, mflops_v1,
               time_v3, speedup_v3, eff_v3, mflops_v3);
        
        free(mat);
        free(rhs);
        free(lhs1);
        free(lhs3);
    }
    return 0;
}
