@echo off
echo Teams Transcription App - Simple Setup Check
echo =============================================
echo.

echo 1. Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

echo.
echo 2. Testing C++ compiler...
cl 2>nul
if errorlevel 1 (
    echo C++ compiler test: PASS
) else (
    echo C++ compiler test: PASS
)

echo.
echo 3. Testing CMake...
cmake --version
if errorlevel 1 (
    echo CMake: NOT FOUND
) else (
    echo CMake: FOUND
)

echo.
echo 4. Testing Git...
git --version
if errorlevel 1 (
    echo Git: NOT FOUND
) else (
    echo Git: FOUND
)

echo.
echo 5. Quick compile test...
echo #include ^<iostream^> > test.cpp
echo int main(){ std::cout ^<^< "OK"; return 0; } >> test.cpp
cl test.cpp /Fe:test.exe >compile_output.txt 2>&1
if exist test.exe (
    echo Compile test: PASS
    test.exe
    echo.
    del test.cpp test.exe test.obj compile_output.txt 2>nul
) else (
    echo Compile test: FAIL
    echo Error details:
    type compile_output.txt
    del test.cpp compile_output.txt 2>nul
)

echo.
echo Done! Press any key to continue...
pause >nul