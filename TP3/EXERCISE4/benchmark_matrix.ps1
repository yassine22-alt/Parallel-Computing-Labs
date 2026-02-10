# Benchmark script for matrix multiplication
# PowerShell script

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Matrix Multiplication Performance Benchmark" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

$size = 1000

# Compile programs
Write-Host "Compiling programs..." -ForegroundColor Yellow
gcc -fopenmp -O3 -Wall -o matrix_mult matrix_mult.c
gcc -fopenmp -O3 -Wall -o matrix_mult_sched matrix_mult_sched.c

Write-Host "`n--- Testing with different thread counts ---`n" -ForegroundColor Cyan

$threads = @(1, 2, 4, 8)
$results = @()

foreach ($t in $threads) {
    $env:OMP_NUM_THREADS = $t
    Write-Host "Testing with $t thread(s)..." -ForegroundColor Yellow
    $output = .\matrix_mult.exe $size
    Write-Host $output
    $results += $output
}

Write-Host "`n--- Testing different scheduling strategies ---`n" -ForegroundColor Cyan
Write-Host "Threads,Schedule,Chunk,Time(s),GFLOPS" -ForegroundColor Yellow

$env:OMP_NUM_THREADS = 4

# STATIC scheduling
.\matrix_mult_sched.exe $size $size "static" 0
.\matrix_mult_sched.exe $size $size "static" 10
.\matrix_mult_sched.exe $size $size "static" 50
.\matrix_mult_sched.exe $size $size "static" 100

# DYNAMIC scheduling
.\matrix_mult_sched.exe $size $size "dynamic" 10
.\matrix_mult_sched.exe $size $size "dynamic" 50
.\matrix_mult_sched.exe $size $size "dynamic" 100

# GUIDED scheduling
.\matrix_mult_sched.exe $size $size "guided" 0
.\matrix_mult_sched.exe $size $size "guided" 10
.\matrix_mult_sched.exe $size $size "guided" 50

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Benchmark Complete!" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green
