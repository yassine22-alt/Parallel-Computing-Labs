#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000000
#define U 2

// Prevent compiler from optimizing away the computation
void escape(void *p) { __asm__ volatile("" : : "g"(p) : "memory"); }

void test_double(FILE *fp) {
    double *a = malloc(N * sizeof(double));
    double sum = 0.0;
    for (int i = 0; i < N; i++) a[i] = 1.0;
    
    double start = (double)clock() / CLOCKS_PER_SEC;
    for (int i = 0; i < N; i += 2)
        sum += a[i] + a[i+1];
    escape(&sum);
    double end = (double)clock() / CLOCKS_PER_SEC;
    
    double time_ms = (end - start) * 1000;
    printf("double: U=%d, Time=%.3f ms\n", U, time_ms);
    fprintf(fp, "double,%d,%.3f\n", U, time_ms);
    free(a);
}

void test_float(FILE *fp) {
    float *a = malloc(N * sizeof(float));
    float sum = 0.0;
    for (int i = 0; i < N; i++) a[i] = 1.0;
    
    double start = (double)clock() / CLOCKS_PER_SEC;
    for (int i = 0; i < N; i += 2)
        sum += a[i] + a[i+1];
    escape(&sum);
    double end = (double)clock() / CLOCKS_PER_SEC;
    
    double time_ms = (end - start) * 1000;
    printf("float:  U=%d, Time=%.3f ms\n", U, time_ms);
    fprintf(fp, "float,%d,%.3f\n", U, time_ms);
    free(a);
}

void test_int(FILE *fp) {
    int *a = malloc(N * sizeof(int));
    int sum = 0;
    for (int i = 0; i < N; i++) a[i] = 1;
    
    double start = (double)clock() / CLOCKS_PER_SEC;
    for (int i = 0; i < N; i += 2)
        sum += a[i] + a[i+1];
    escape(&sum);
    double end = (double)clock() / CLOCKS_PER_SEC;
    
    double time_ms = (end - start) * 1000;
    printf("int:    U=%d, Time=%.3f ms\n", U, time_ms);
    fprintf(fp, "int,%d,%.3f\n", U, time_ms);
    free(a);
}

void test_short(FILE *fp) {
    short *a = malloc(N * sizeof(short));
    int sum = 0;
    for (int i = 0; i < N; i++) a[i] = 1;
    
    double start = (double)clock() / CLOCKS_PER_SEC;
    for (int i = 0; i < N; i += 2)
        sum += a[i] + a[i+1];
    escape(&sum);
    double end = (double)clock() / CLOCKS_PER_SEC;
    
    double time_ms = (end - start) * 1000;
    printf("short:  U=%d, Time=%.3f ms\n", U, time_ms);
    fprintf(fp, "short,%d,%.3f\n", U, time_ms);
    free(a);
}

int main(int argc, char *argv[]) {
    char opt_level[10] = "O0";
    if (argc > 1) sprintf(opt_level, "%s", argv[1]);
    
    char filename[100];
    sprintf(filename, "results_U%d_%s.csv", U, opt_level);
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "Type,UnrollFactor,Time_ms\n");
    
    printf("Testing U=%d with %s\n", U, opt_level);
    test_double(fp);
    test_float(fp);
    test_int(fp);
    test_short(fp);
    
    fclose(fp);
    return 0;
}
