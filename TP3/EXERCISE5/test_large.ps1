# Test with larger problem size
Write-Host "`nTesting with LARGE problem (1000x1000):" -ForegroundColor Green
Write-Host "=========================================`n" -ForegroundColor Green

Write-Host "Sequential:" -ForegroundColor Yellow
$seq = .\jacobi_sequential_large.exe 2>&1 | Out-String
Write-Host $seq

if ($seq -match "CPU time\s+:\s+([\d.E+-]+)") {
    $baseline = [double]$matches[1]
    Write-Host "Baseline time: $baseline seconds`n" -ForegroundColor Cyan
}

$threads = @(1, 2, 4, 8, 16)
Write-Host "`nThreads | CPU Time(s) | Speedup | Efficiency" -ForegroundColor Yellow
Write-Host "--------|-------------|---------|------------" -ForegroundColor Yellow

foreach ($t in $threads) {
    $env:OMP_NUM_THREADS = $t
    $out = .\jacobi_parallel_large.exe 2>&1 | Out-String
    
    if ($out -match "CPU time\s+:\s+([\d.E+-]+)") {
        $time = [double]$matches[1]
        $speedup = $baseline / $time
        $efficiency = $speedup / $t
        Write-Host ("{0,7} | {1,11:F6} | {2,7:F3} | {3,10:F3}" -f $t, $time, $speedup, $efficiency)
    }
}
