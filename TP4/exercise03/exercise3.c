#include <stdio.h>
#include <math.h>
#include <omp.h>

#define N 1000000

void task_light(int n) {
    double x = 0.0;
    for (int i = 0; i < n; i++) {
        x += sin(i * 0.001);
    }
}

void task_moderate(int n) {
    double x = 0.0;
    for (int i = 0; i < 5*n; i++) {
        x += sqrt(i * 0.5) * cos(i * 0.001);
    }
}

void task_heavy(int n) {
    double x = 0.0;
    for (int i = 0; i < 20*n; i++) {
        x += sqrt(i * 0.5) * cos(i * 0.001) * sin(i * 0.0001);
    }
}

// Split heavy task into chunks for better load balancing
void task_heavy_chunk(int n, int start, int end) {
    double x = 0.0;
    for (int i = start; i < end; i++) {
        x += sqrt(i * 0.5) * cos(i * 0.001) * sin(i * 0.0001);
    }
}

int main() {
    double start_time, end_time;
    
    printf("Task workload analysis:\n");
    printf("- Light task:    %d iterations (1x)\n", N);
    printf("- Moderate task: %d iterations (5x)\n", 5*N);
    printf("- Heavy task:    %d iterations (20x)\n\n", 20*N);

    // ==========================================
    // 1. SEQUENTIAL EXECUTION (baseline)
    // ==========================================
    printf("=== SEQUENTIAL EXECUTION ===\n");
    start_time = omp_get_wtime();
    
    task_light(N);
    task_moderate(N);
    task_heavy(N);
    
    end_time = omp_get_wtime();
    double time_seq = end_time - start_time;
    printf("Execution time: %.4f seconds\n\n", time_seq);

    // ==========================================
    // 2. NAIVE PARALLEL SECTIONS (poor load balancing)
    // ==========================================
    printf("=== NAIVE PARALLEL SECTIONS (3 sections, 3 tasks) ===\n");
    
    start_time = omp_get_wtime();
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_light(N);
            double task_end = omp_get_wtime();
            printf("Thread %d: Light task completed in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_moderate(N);
            double task_end = omp_get_wtime();
            printf("Thread %d: Moderate task completed in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_heavy(N);
            double task_end = omp_get_wtime();
            printf("Thread %d: Heavy task completed in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
    }
    // Implicit barrier: all threads wait for the slowest (heavy task)
    
    end_time = omp_get_wtime();
    double time_naive = end_time - start_time;
    printf("Total execution time: %.4f seconds\n", time_naive);
    printf("Speedup: %.2fx\n", time_seq / time_naive);
    printf("Efficiency: %.1f%% (threads wait idle for heavy task)\n\n", 
           (time_seq / time_naive / 3.0) * 100);

    // ==========================================
    // 3. OPTIMIZED: Split heavy task into multiple sections
    // ==========================================
    printf("=== OPTIMIZED PARALLEL SECTIONS (split heavy task) ===\n");
    printf("Strategy: Split heavy task into 4 chunks for better distribution\n");
    
    start_time = omp_get_wtime();
    
    int heavy_total = 20 * N;
    int num_chunks = 4;
    int chunk_size = heavy_total / num_chunks;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_light(N);
            double task_end = omp_get_wtime();
            printf("Thread %d: Light task in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_moderate(N);
            double task_end = omp_get_wtime();
            printf("Thread %d: Moderate task in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        // Split heavy task into 4 sections
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_heavy_chunk(N, 0, chunk_size);
            double task_end = omp_get_wtime();
            printf("Thread %d: Heavy chunk 1/4 in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_heavy_chunk(N, chunk_size, 2*chunk_size);
            double task_end = omp_get_wtime();
            printf("Thread %d: Heavy chunk 2/4 in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_heavy_chunk(N, 2*chunk_size, 3*chunk_size);
            double task_end = omp_get_wtime();
            printf("Thread %d: Heavy chunk 3/4 in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
        
        #pragma omp section
        {
            double task_start = omp_get_wtime();
            task_heavy_chunk(N, 3*chunk_size, heavy_total);
            double task_end = omp_get_wtime();
            printf("Thread %d: Heavy chunk 4/4 in %.4f seconds\n", 
                   omp_get_thread_num(), task_end - task_start);
        }
    }
    
    end_time = omp_get_wtime();
    double time_optimized = end_time - start_time;
    printf("Total execution time: %.4f seconds\n", time_optimized);
    printf("Speedup vs sequential: %.2fx\n", time_seq / time_optimized);
    printf("Speedup vs naive: %.2fx\n\n", time_naive / time_optimized);

    // ==========================================
    // 4. ALTERNATIVE: Use parallel for instead of sections
    // ==========================================
    printf("=== ALTERNATIVE: Using parallel for with dynamic scheduling ===\n");
    printf("Strategy: Let OpenMP handle load balancing dynamically\n");
    
    start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                double task_start = omp_get_wtime();
                task_light(N);
                double task_end = omp_get_wtime();
                printf("Thread %d: Light task in %.4f seconds\n", 
                       omp_get_thread_num(), task_end - task_start);
            }
            
            #pragma omp task
            {
                double task_start = omp_get_wtime();
                task_moderate(N);
                double task_end = omp_get_wtime();
                printf("Thread %d: Moderate task in %.4f seconds\n", 
                       omp_get_thread_num(), task_end - task_start);
            }
            
            #pragma omp task
            {
                double task_start = omp_get_wtime();
                task_heavy(N);
                double task_end = omp_get_wtime();
                printf("Thread %d: Heavy task in %.4f seconds\n", 
                       omp_get_thread_num(), task_end - task_start);
            }
            
            #pragma omp taskwait
        }
    }
    
    end_time = omp_get_wtime();
    double time_tasks = end_time - start_time;
    printf("Total execution time: %.4f seconds\n", time_tasks);
    printf("Speedup vs sequential: %.2fx\n\n", time_seq / time_tasks);

    // ==========================================
    // SUMMARY
    // ==========================================
    printf("=== PERFORMANCE SUMMARY ===\n");
    printf("Sequential:          %.4f s (baseline)\n", time_seq);
    printf("Naive sections:      %.4f s (%.2fx speedup)\n", time_naive, time_seq/time_naive);
    printf("Optimized sections:  %.4f s (%.2fx speedup)\n", time_optimized, time_seq/time_optimized);
    printf("Task-based:          %.4f s (%.2fx speedup)\n", time_tasks, time_seq/time_tasks);
    printf("\nNumber of threads: %d\n", omp_get_max_threads());


    return 0;
}
