@echo off
echo ============================================
echo Teams Transcription App - Setup Verification
echo ============================================
echo.

echo Checking Visual Studio 2022...
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo ✅ Visual Studio 2022 Community found
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    echo ✅ Visual Studio 2022 Professional found
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    echo ✅ Visual Studio 2022 Enterprise found
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
) else (
    echo ❌ Visual Studio 2022 not found
    echo Please install Visual Studio 2022 with C++ workload
    goto :end
)

echo.
echo Checking C++ compiler...
cl >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ C++ compiler (cl.exe) available
) else (
    echo ❌ C++ compiler not found
    echo Please install C++ workload in Visual Studio
)

echo.
echo Checking CMake...
cmake --version >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ CMake found
    cmake --version
) else (
    echo ❌ CMake not found
    echo Please install CMake or use Visual Studio's built-in CMake
)

echo.
echo Checking Git...
git --version >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ Git found
    git --version
) else (
    echo ❌ Git not found
    echo Please install Git for Windows
)

echo.
echo Checking Windows SDK...
if exist "C:\Program Files (x86)\Windows Kits\10\Include" (
    echo ✅ Windows SDK found
    dir "C:\Program Files (x86)\Windows Kits\10\Include" /b | findstr /R "^10\."
) else (
    echo ❌ Windows SDK not found
    echo Please install Windows 10/11 SDK
)

echo.
echo Checking vcpkg...
if exist "C:\vcpkg\vcpkg.exe" (
    echo ✅ vcpkg found at C:\vcpkg
) else if exist "C:\tools\vcpkg\vcpkg.exe" (
    echo ✅ vcpkg found at C:\tools\vcpkg
) else (
    echo ❌ vcpkg not found
    echo Run: git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    echo Then: C:\vcpkg\bootstrap-vcpkg.bat
)

echo.
echo Checking project dependencies...
if exist "C:\vcpkg\installed\x64-windows\include\nlohmann\json.hpp" (
    echo ✅ nlohmann/json library found
) else (
    echo ❌ nlohmann/json library not found
    echo Run: C:\vcpkg\vcpkg install nlohmann-json:x64-windows
)

echo.
echo ============================================
echo Testing basic compilation...
echo ============================================

echo Creating test file...
echo #include ^<iostream^> > test_compile.cpp
echo int main() { >> test_compile.cpp
echo     std::cout ^<^< "Hello, Visual Studio!" ^<^< std::endl; >> test_compile.cpp
echo     return 0; >> test_compile.cpp
echo } >> test_compile.cpp

echo Compiling test file...
cl test_compile.cpp /Fe:test_compile.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo ✅ Basic compilation successful
    echo Running test...
    test_compile.exe
    del test_compile.cpp test_compile.exe test_compile.obj >nul 2>&1
) else (
    echo ❌ Compilation failed
    echo Please check Visual Studio C++ installation
)

:end
echo.
echo ============================================
echo Setup verification complete!
echo ============================================
pause