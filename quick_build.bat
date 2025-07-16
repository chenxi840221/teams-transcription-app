@echo off
echo Setting up Visual Studio environment...

call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo Building with simple logger...
cmake --build build --config Debug

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Build successful! 
    echo You can now run: build\output\Debug\TeamsTranscriptionApp.exe
    echo Check debug.log for detailed logging output
) else (
    echo.
    echo ❌ Build failed. Check errors above.
)

pause