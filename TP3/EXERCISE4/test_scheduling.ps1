# Test different scheduling strategies
# PowerShell script

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Scheduling Strategy Comparison" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

$env:OMP_NUM_THREADS = 4
$size = 1000

Write-Host "Testing with 4 threads, matrix size ${size}x${size}`n" -ForegroundColor Yellow
Write-Host "Schedule  | Chunk | Time(s)  | GFLOPS" -ForegroundColor Yellow
Write-Host "----------|-------|----------|--------" -ForegroundColor Yellow

# STATIC scheduling with different chunk sizes
Write-Host "`nSTATIC Scheduling:" -ForegroundColor Cyan
.\matrix_mult_sched.exe $size $size "static" 0
.\matrix_mult_sched.exe $size $size "static" 10
.\matrix_mult_sched.exe $size $size "static" 50
.\matrix_mult_sched.exe $size $size "static" 100

# DYNAMIC scheduling with different chunk sizes
Write-Host "`nDYNAMIC Scheduling:" -ForegroundColor Cyan
.\matrix_mult_sched.exe $size $size "dynamic" 10
.\matrix_mult_sched.exe $size $size "dynamic" 50
.\matrix_mult_sched.exe $size $size "dynamic" 100
.\matrix_mult_sched.exe $size $size "dynamic" 200

# GUIDED scheduling
Write-Host "`nGUIDED Scheduling:" -ForegroundColor Cyan
.\matrix_mult_sched.exe $size $size "guided" 0
.\matrix_mult_sched.exe $size $size "guided" 10
.\matrix_mult_sched.exe $size $size "guided" 50

Write-Host "`n========================================" -ForegroundColor Green
