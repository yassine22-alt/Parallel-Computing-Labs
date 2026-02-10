# Jacobi Method Speedup and Efficiency Analysis
# PowerShell script

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Jacobi Method - Speedup & Efficiency" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

# Compile programs
Write-Host "Compiling programs..." -ForegroundColor Yellow
gcc -fopenmp -O3 -Wall -o jacobi_sequential jacobi_sequential.c -lm
gcc -fopenmp -O3 -Wall -o jacobi_parallel jacobi_parallel.c -lm

$threads_list = @(1, 2, 4, 8, 16)
$times = @{}

Write-Host "`n--- Running Sequential Version ---`n" -ForegroundColor Cyan
$seq_output = .\jacobi_sequential.exe 2>&1 | Out-String
Write-Host $seq_output

if ($seq_output -match "CPU time\s+:\s+([\d.E+-]+)") {
    $baseline_time = [double]$matches[1]
    Write-Host "Sequential CPU time: $baseline_time seconds`n" -ForegroundColor Yellow
} else {
    Write-Host "Failed to extract sequential time!" -ForegroundColor Red
    exit 1
}

Write-Host "`n--- Running Parallel Version with Different Thread Counts ---`n" -ForegroundColor Cyan

foreach ($t in $threads_list) {
    $env:OMP_NUM_THREADS = $t
    Write-Host "Testing with $t thread(s)..." -ForegroundColor Yellow
    
    $output = .\jacobi_parallel.exe 2>&1 | Out-String
    Write-Host $output
    
    if ($output -match "CPU time\s+:\s+([\d.E+-]+)") {
        $time = [double]$matches[1]
        $times[$t] = $time
    }
}

# Calculate speedup and efficiency
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Performance Analysis" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

Write-Host "Threads | Time(s)     | Speedup | Efficiency" -ForegroundColor Yellow
Write-Host "--------|-------------|---------|------------" -ForegroundColor Yellow

foreach ($t in $threads_list) {
    $time = $times[$t]
    $speedup = $baseline_time / $time
    $efficiency = $speedup / $t
    
    Write-Host ("{0,7} | {1,11:E3} | {2,7:F3} | {3,10:F3}" -f $t, $time, $speedup, $efficiency)
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "CSV Data for Plotting:" -ForegroundColor Yellow
Write-Host "Threads,Time,Speedup,Efficiency"
foreach ($t in $threads_list) {
    $time = $times[$t]
    $speedup = $baseline_time / $time
    $efficiency = $speedup / $t
    Write-Host "$t,$time,$speedup,$efficiency"
}
Write-Host "========================================`n" -ForegroundColor Green
