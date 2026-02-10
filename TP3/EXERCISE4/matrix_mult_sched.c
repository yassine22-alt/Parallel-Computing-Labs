#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int m = 1000;
    int n = 1000;
    char schedule_type[20] = "static";
    int chunk_size = 0;
    
    // Parse arguments: ./program [size] [schedule] [chunk]
    if (argc > 1) m = atoi(argv[1]);
    if (argc > 2) n = atoi(argv[2]);
    if (argc > 3) strcpy(schedule_type, argv[3]);
    if (argc > 4) chunk_size = atoi(argv[4]);
    
    // Allocate memory
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
    
    // Matrix multiplication with scheduling
    double start_time = omp_get_wtime();
    
    if (strcmp(schedule_type, "static") == 0) {
        if (chunk_size > 0) {
            #pragma omp parallel for collapse(2) schedule(static, chunk_size)
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < n; k++) {
                        c[i * m + j] += a[i * n + k] * b[k * m + j];
                    }
                }
            }
        } else {
            #pragma omp parallel for collapse(2) schedule(static)
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < n; k++) {
                        c[i * m + j] += a[i * n + k] * b[k * m + j];
                    }
                }
            }
        }
    } else if (strcmp(schedule_type, "dynamic") == 0) {
        int chunk = chunk_size > 0 ? chunk_size : 10;
        #pragma omp parallel for collapse(2) schedule(dynamic, chunk)
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < n; k++) {
                    c[i * m + j] += a[i * n + k] * b[k * m + j];
                }
            }
        }
    } else if (strcmp(schedule_type, "guided") == 0) {
        if (chunk_size > 0) {
            #pragma omp parallel for collapse(2) schedule(guided, chunk_size)
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < n; k++) {
                        c[i * m + j] += a[i * n + k] * b[k * m + j];
                    }
                }
            }
        } else {
            #pragma omp parallel for collapse(2) schedule(guided)
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < m; j++) {
                    for (int k = 0; k < n; k++) {
                        c[i * m + j] += a[i * n + k] * b[k * m + j];
                    }
                }
            }
        }
    }
    
    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;
    
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp master
        num_threads = omp_get_num_threads();
    }
    
    printf("%d,%s,%d,%.6f,%.3f\n", 
           num_threads, 
           schedule_type, 
           chunk_size,
           elapsed,
           (2.0 * m * m * n) / (elapsed * 1e9));
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
