@echo off
echo Setting up Visual Studio environment...

:: Try different VS paths
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    goto :build
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat"
    goto :build
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
    goto :build
)

echo ERROR: Could not find Visual Studio Developer Command Prompt
echo Please run this from a VS Developer Command Prompt instead:
echo   cmake --build build --config Debug
pause
exit /b 1

:build
echo Building project with enhanced logging...
cmake --build build --config Debug

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! 
    echo You can now run: build\output\Debug\TeamsTranscriptionApp.exe
    echo Check debug.log for detailed logging output
) else (
    echo.
    echo Build failed. Please check the errors above.
)

pause