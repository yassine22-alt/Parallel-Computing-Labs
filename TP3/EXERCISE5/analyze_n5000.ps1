# Jacobi N=5000 Performance Analysis
Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Jacobi Method - N=5000 Analysis" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

Write-Host "Running Sequential Version..." -ForegroundColor Yellow
$seq = .\jacobi_seq_5000.exe 2>&1 | Out-String
Write-Host $seq

if ($seq -match "CPU time\s+:\s+([\d.E+-]+)") {
    $baseline = [double]$matches[1]
    Write-Host "`nBaseline CPU time: $baseline seconds`n" -ForegroundColor Cyan
}

$threads = @(1, 2, 4, 8, 16)
$times = @{}

Write-Host "`nRunning Parallel Versions...`n" -ForegroundColor Yellow

foreach ($t in $threads) {
    $env:OMP_NUM_THREADS = $t
    Write-Host "Testing with $t thread(s)..." -ForegroundColor Cyan
    $out = .\jacobi_par_5000.exe 2>&1 | Out-String
    
    if ($out -match "CPU time\s+:\s+([\d.E+-]+)") {
        $time = [double]$matches[1]
        $times[$t] = $time
        Write-Host "  CPU time: $time seconds" -ForegroundColor White
    }
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Results Summary (N=5000)" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

Write-Host "Threads | Time(s)  | Speedup | Efficiency" -ForegroundColor Yellow
Write-Host "--------|----------|---------|------------" -ForegroundColor Yellow

foreach ($t in $threads) {
    $time = $times[$t]
    $speedup = $baseline / $time
    $efficiency = $speedup / $t
    Write-Host ("{0,7} | {1,8:F4} | {2,7:F3} | {3,10:F3}" -f $t, $time, $speedup, $efficiency)
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "CSV Data:" -ForegroundColor Yellow
Write-Host "Threads,Time,Speedup,Efficiency"
foreach ($t in $threads) {
    $time = $times[$t]
    $speedup = $baseline / $time
    $efficiency = $speedup / $t
    Write-Host "$t,$time,$speedup,$efficiency"
}
Write-Host "========================================`n" -ForegroundColor Green
