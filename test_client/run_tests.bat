@echo off
REM Windows batch script to run tests

echo Starting Test Client for First Client-Server
echo.

if "%1"=="" (
    echo Usage: run_tests.bat [script_name] [options]
    echo.
    echo Examples:
    echo   run_tests.bat basic_test.txt
    echo   run_tests.bat advanced_test.txt --verbose
    echo   run_tests.bat stress_test.txt
    echo.
    echo Available test scripts:
    dir /b *.txt
    exit /b 1
)

set SCRIPT_FILE=%1
set OPTIONS=%2 %3 %4 %5 %6 %7 %8 %9

if not exist "%SCRIPT_FILE%" (
    echo Error: Script file '%SCRIPT_FILE%' not found.
    exit /b 1
)

echo Running test script: %SCRIPT_FILE%
echo Options: %OPTIONS%
echo.

REM Navigate to build directory and run the test client
cd ..\build\test_client\Debug
if errorlevel 1 (
    echo Error: Could not find build directory. Make sure the project is built.
    exit /b 1
)

test_client.exe --script "..\..\..\test_client\%SCRIPT_FILE%" %OPTIONS%

REM Return to original directory
cd ..\..\..\test_client

echo.
echo Test execution completed with exit code: %errorlevel%
