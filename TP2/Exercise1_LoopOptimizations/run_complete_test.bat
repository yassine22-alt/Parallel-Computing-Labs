@echo off
echo ========================================
echo Complete Loop Unrolling Analysis
echo Testing U=1,2,4,8,16,32 for all types
echo Both -O0 and -O2 optimization levels
echo ========================================
echo.

set unroll_factors=1 2 4 8 16 32

echo [PHASE 1] Testing with -O0 (no optimization)
echo ========================================
for %%u in (%unroll_factors%) do (
    echo Compiling U=%%u with -O0...
    gcc -O0 sum_u%%u_all_types.c -o sum_u%%u_O0.exe
    if %errorlevel% equ 0 (
        echo Running U=%%u -O0...
        sum_u%%u_O0.exe O0
        echo.
    ) else (
        echo ERROR: Compilation failed for U=%%u -O0
    )
)

echo.
echo [PHASE 2] Testing with -O2 (optimization enabled)
echo ========================================
for %%u in (%unroll_factors%) do (
    echo Compiling U=%%u with -O2...
    gcc -O2 sum_u%%u_all_types.c -o sum_u%%u_O2.exe
    if %errorlevel% equ 0 (
        echo Running U=%%u -O2...
        sum_u%%u_O2.exe O2
        echo.
    ) else (
        echo ERROR: Compilation failed for U=%%u -O2
    )
)

echo ========================================
echo All tests complete!
echo ========================================
echo.
echo Results saved to:
for %%u in (%unroll_factors%) do (
    echo   - results_U%%u_O0.csv
    echo   - results_U%%u_O2.csv
)
echo ========================================
pause
