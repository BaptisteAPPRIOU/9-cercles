@echo off
REM Concurrent test runner - launches 4 test clients simultaneously

echo Starting Concurrent Test with 4 Clients
echo ========================================
echo.

REM Check if test_client.exe exists
if not exist "..\build\test_client\Debug\test_client.exe" (
    echo Error: test_client.exe not found. Please build the project first.
    echo Expected location: ..\build\test_client\Debug\test_client.exe
    pause
    exit /b 1
)

REM Create a temporary directory for test results
if not exist "test_results" mkdir test_results

echo Starting 4 concurrent clients...
echo.

REM Start 4 clients in parallel, each with a unique identifier
start "Client 1" cmd /c "..\build\test_client\Debug\test_client.exe --script concurrent_test.txt --verbose > test_results\client1_output.log 2>&1 && echo Client 1 COMPLETED > test_results\client1_done.txt"

start "Client 2" cmd /c "..\build\test_client\Debug\test_client.exe --script concurrent_test.txt --verbose > test_results\client2_output.log 2>&1 && echo Client 2 COMPLETED > test_results\client2_done.txt"

start "Client 3" cmd /c "..\build\test_client\Debug\test_client.exe --script concurrent_test.txt --verbose > test_results\client3_output.log 2>&1 && echo Client 3 COMPLETED > test_results\client3_done.txt"

start "Client 4" cmd /c "..\build\test_client\Debug\test_client.exe --script concurrent_test.txt --verbose > test_results\client4_output.log 2>&1 && echo Client 4 COMPLETED > test_results\client4_done.txt"

echo All clients started. Waiting for completion...
echo.

REM Wait for all clients to complete (check for completion files)
:wait_loop
timeout /t 1 /nobreak >nul 2>&1

set /a completed_count=0
if exist "test_results\client1_done.txt" set /a completed_count+=1
if exist "test_results\client2_done.txt" set /a completed_count+=1
if exist "test_results\client3_done.txt" set /a completed_count+=1
if exist "test_results\client4_done.txt" set /a completed_count+=1

echo Completed clients: %completed_count%/4
if %completed_count% LSS 4 goto wait_loop

echo.
echo ========================================
echo All clients completed! Showing results:
echo ========================================
echo.

REM Display results from each client
for %%i in (1 2 3 4) do (
    echo --- Client %%i Results ---
    if exist "test_results\client%%i_output.log" (
        type "test_results\client%%i_output.log"
    ) else (
        echo No output file found for client %%i
    )
    echo.
)

echo ========================================
echo Concurrent test completed!
echo Check test_results folder for detailed logs.
echo ========================================

pause
