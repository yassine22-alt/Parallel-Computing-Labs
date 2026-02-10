#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000000
#define U 32

// Prevent compiler from optimizing away the computation
void escape(void *p) { __asm__ volatile("" : : "g"(p) : "memory"); }

void test_double(FILE *fp) {
    double *a = malloc(N * sizeof(double));
    double sum = 0.0;
    for (int i = 0; i < N; i++) a[i] = 1.0;
    
    double start = (double)clock() / CLOCKS_PER_SEC;
    for (int i = 0; i < N; i += 32)
        sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
               a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15] +
               a[i+16] + a[i+17] + a[i+18] + a[i+19] + a[i+20] + a[i+21] + a[i+22] + a[i+23] +
               a[i+24] + a[i+25] + a[i+26] + a[i+27] + a[i+28] + a[i+29] + a[i+30] + a[i+31];
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
    for (int i = 0; i < N; i += 32)
        sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
               a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15] +
               a[i+16] + a[i+17] + a[i+18] + a[i+19] + a[i+20] + a[i+21] + a[i+22] + a[i+23] +
               a[i+24] + a[i+25] + a[i+26] + a[i+27] + a[i+28] + a[i+29] + a[i+30] + a[i+31];
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
    for (int i = 0; i < N; i += 32)
        sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
               a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15] +
               a[i+16] + a[i+17] + a[i+18] + a[i+19] + a[i+20] + a[i+21] + a[i+22] + a[i+23] +
               a[i+24] + a[i+25] + a[i+26] + a[i+27] + a[i+28] + a[i+29] + a[i+30] + a[i+31];
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
    for (int i = 0; i < N; i += 32)
        sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
               a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15] +
               a[i+16] + a[i+17] + a[i+18] + a[i+19] + a[i+20] + a[i+21] + a[i+22] + a[i+23] +
               a[i+24] + a[i+25] + a[i+26] + a[i+27] + a[i+28] + a[i+29] + a[i+30] + a[i+31];
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
