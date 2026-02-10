# Speedup and Efficiency Analysis for Matrix Multiplication
# PowerShell script

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Speedup & Efficiency Analysis" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

$size = 1000
$threads_list = @(1, 2, 4, 8, 16)
$times = @{}

Write-Host "Running benchmarks with matrix size: ${size}x${size}`n" -ForegroundColor Yellow

# Run tests for each thread count
foreach ($t in $threads_list) {
    $env:OMP_NUM_THREADS = $t
    Write-Host "Testing with $t thread(s)..." -ForegroundColor Cyan
    
    $output = .\matrix_mult.exe $size 2>&1 | Out-String
    
    # Extract execution time
    if ($output -match "Execution time:\s+([\d.]+)") {
        $time = [double]$matches[1]
        $times[$t] = $time
        Write-Host "  Time: $time seconds" -ForegroundColor White
    }
}

# Calculate speedup and efficiency
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Results Summary" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

$baseline_time = $times[1]

Write-Host "Threads | Time(s)  | Speedup | Efficiency" -ForegroundColor Yellow
Write-Host "--------|----------|---------|------------" -ForegroundColor Yellow

foreach ($t in $threads_list) {
    $time = $times[$t]
    $speedup = $baseline_time / $time
    $efficiency = $speedup / $t
    
    Write-Host ("{0,7} | {1,8:F6} | {2,7:F3} | {3,10:F3}" -f $t, $time, $speedup, $efficiency)
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Speedup = T(1) / T(n)" -ForegroundColor Gray
Write-Host "Efficiency = Speedup / n  (ideal = 1.0)" -ForegroundColor Gray
Write-Host "========================================`n" -ForegroundColor Green

# Generate data for plotting
Write-Host "`nData for plotting (CSV format):" -ForegroundColor Yellow
Write-Host "Threads,Time,Speedup,Efficiency"
foreach ($t in $threads_list) {
    $time = $times[$t]
    $speedup = $baseline_time / $time
    $efficiency = $speedup / $t
    Write-Host "$t,$time,$speedup,$efficiency"
}
