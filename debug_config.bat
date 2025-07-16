@echo off
echo Debugging Configuration Issue
echo ==============================

echo Current directory:
cd

echo.
echo Contents of current directory:
dir /b

echo.
echo Looking for config directory:
if exist config (
    echo CONFIG DIRECTORY FOUND
    dir config /b
) else (
    echo CONFIG DIRECTORY NOT FOUND
)

echo.
echo Looking for config\settings.json:
if exist config\settings.json (
    echo CONFIG FILE FOUND
    echo File size:
    dir config\settings.json
) else (
    echo CONFIG FILE NOT FOUND
)

echo.
echo Looking for executable:
if exist Debug\TeamsTranscriptionApp.exe (
    echo EXECUTABLE FOUND
) else (
    echo EXECUTABLE NOT FOUND
)

echo.
echo Trying to run application with output redirection:
Debug\TeamsTranscriptionApp.exe > app_output.txt 2>&1
echo Exit code: %errorlevel%

echo.
echo Application output:
if exist app_output.txt (
    type app_output.txt
) else (
    echo No output file created
)

pause