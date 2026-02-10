#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10000000
#define TYPE short
#define TYPE_NAME "short"

void test_unroll(TYPE *a, int U, FILE *fp) {
    int sum = 0;  // Use int to avoid overflow
    double start, end;
    
    start = (double)clock() / CLOCKS_PER_SEC;
    
    int i;
    switch(U) {
        case 1:
            for (i = 0; i < N; i++)
                sum += a[i];
            break;
        case 2:
            for (i = 0; i < N; i += 2)
                sum += a[i] + a[i+1];
            break;
        case 3:
            for (i = 0; i < N; i += 3)
                sum += a[i] + a[i+1] + a[i+2];
            break;
        case 4:
            for (i = 0; i < N; i += 4)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3];
            break;
        case 5:
            for (i = 0; i < N; i += 5)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4];
            break;
        case 6:
            for (i = 0; i < N; i += 6)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5];
            break;
        case 7:
            for (i = 0; i < N; i += 7)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6];
            break;
        case 8:
            for (i = 0; i < N; i += 8)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7];
            break;
        case 16:
            for (i = 0; i < N; i += 16)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
                       a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15];
            break;
        case 32:
            for (i = 0; i < N; i += 32)
                sum += a[i] + a[i+1] + a[i+2] + a[i+3] + a[i+4] + a[i+5] + a[i+6] + a[i+7] +
                       a[i+8] + a[i+9] + a[i+10] + a[i+11] + a[i+12] + a[i+13] + a[i+14] + a[i+15] +
                       a[i+16] + a[i+17] + a[i+18] + a[i+19] + a[i+20] + a[i+21] + a[i+22] + a[i+23] +
                       a[i+24] + a[i+25] + a[i+26] + a[i+27] + a[i+28] + a[i+29] + a[i+30] + a[i+31];
            break;
    }
    
    end = (double)clock() / CLOCKS_PER_SEC;
    double time_ms = (end - start) * 1000;
    
    printf("U=%d: Sum = %d, Time = %.3f ms\n", U, sum, time_ms);
    fprintf(fp, "%d,%.3f\n", U, time_ms);
}

int main(int argc, char *argv[]) {
    char opt_level[10] = "O0";
    if (argc > 1) {
        sprintf(opt_level, "%s", argv[1]);
    }
    
    TYPE *a = malloc(N * sizeof(TYPE));
    
    for (int i = 0; i < N; i++)
        a[i] = 1;
    
    char filename[100];
    sprintf(filename, "results_%s_%s.csv", TYPE_NAME, opt_level);
    FILE *fp = fopen(filename, "w");
    fprintf(fp, "UnrollFactor,Time_ms\n");
    
    printf("========================================\n");
    printf("Type: %s, Optimization: %s\n", TYPE_NAME, opt_level);
    printf("========================================\n");
    
    int unroll_factors[] = {1, 2, 3, 4, 5, 6, 7, 8, 16, 32};
    for (int i = 0; i < 10; i++) {
        test_unroll(a, unroll_factors[i], fp);
    }
    
    fclose(fp);
    free(a);
    
    printf("Results saved to %s\n", filename);
    return 0;
}
