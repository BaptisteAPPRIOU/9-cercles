@echo off
echo Deploying client application...

REM Create deployment directory
if not exist "client_deploy" mkdir client_deploy

REM Copy executable
copy "build\client\client.exe" "client_deploy\"

REM Copy environment configuration
copy ".env" "client_deploy\"

REM Copy required DLLs (if any)
if exist "build\client\*.dll" copy "build\client\*.dll" "client_deploy\"

echo.
echo Client deployment complete!
echo.
echo Files in client_deploy directory:
dir "client_deploy"
echo.
echo To deploy on another machine:
echo 1. Copy the entire client_deploy folder to the target machine
echo 2. Modify the .env file in the client_deploy folder with the correct server IP
echo 3. Run client.exe from the client_deploy folder
echo.
pause