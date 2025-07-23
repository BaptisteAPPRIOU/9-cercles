@echo off
REM Enhanced concurrent test runner with unique client IDs

echo Starting Enhanced Concurrent Test with 4 Unique Clients
echo ====================================================
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

echo Starting 4 concurrent clients with unique IDs...
echo Each client will send messages with their unique identifier.
echo.

REM Start 4 clients in parallel, each with a unique client ID
start "Client A" cmd /c "..\build\test_client\Debug\test_client.exe --client-id ClientA --script unique_client_test.txt --verbose > test_results\clientA_output.log 2>&1 && echo Client A COMPLETED > test_results\clientA_done.txt"

start "Client B" cmd /c "..\build\test_client\Debug\test_client.exe --client-id ClientB --script unique_client_test.txt --verbose > test_results\clientB_output.log 2>&1 && echo Client B COMPLETED > test_results\clientB_done.txt"

start "Client C" cmd /c "..\build\test_client\Debug\test_client.exe --client-id ClientC --script unique_client_test.txt --verbose > test_results\clientC_output.log 2>&1 && echo Client C COMPLETED > test_results\clientC_done.txt"

start "Client D" cmd /c "..\build\test_client\Debug\test_client.exe --client-id ClientD --script unique_client_test.txt --verbose > test_results\clientD_output.log 2>&1 && echo Client D COMPLETED > test_results\clientD_done.txt"

echo All clients started with IDs: ClientA, ClientB, ClientC, ClientD
echo Waiting for completion...
echo.

REM Wait for all clients to complete (check for completion files)
:wait_loop
timeout /t 1 /nobreak >nul 2>&1

set /a completed_count=0
if exist "test_results\clientA_done.txt" set /a completed_count+=1
if exist "test_results\clientB_done.txt" set /a completed_count+=1
if exist "test_results\clientC_done.txt" set /a completed_count+=1
if exist "test_results\clientD_done.txt" set /a completed_count+=1

echo Completed clients: %completed_count%/4
if %completed_count% LSS 4 goto wait_loop

echo.
echo ====================================================
echo All clients completed! Analyzing results:
echo ====================================================
echo.

set /a total_failures=0

REM Display results from each client
for %%i in (A B C D) do (
    echo --- Client%%i Results ---
    if exist "test_results\client%%i_output.log" (
        type "test_results\client%%i_output.log"
        REM Check if the client passed (simple check for "tests passed" in output)
        findstr /C:"Summary:" "test_results\client%%i_output.log" | findstr /C:"4/4 tests passed" >nul
        if errorlevel 1 (
            echo Client%%i: FAILED
            set /a total_failures+=1
        ) else (
            echo Client%%i: PASSED
        )
    ) else (
        echo No output file found for client %%i
        set /a total_failures+=1
    )
    echo.
)

echo ====================================================
echo CONCURRENT TEST SUMMARY
echo ====================================================
if %total_failures% EQU 0 (
    echo Result: ALL CLIENTS PASSED
    echo Your server successfully handled 4 concurrent connections!
) else (
    echo Result: %total_failures% CLIENT(S) FAILED
    echo Your server may have issues handling concurrent connections.
)
echo ====================================================
echo.
echo Detailed logs are available in the test_results folder.
echo.

pause
