# 🚀 Development Environment Setup Guide

**Target:** Windows C++ Development Environment  
**Project:** Teams Audio Transcription Application  
**Date:** July 14, 2025  

---

## 📋 Prerequisites Check

You're currently in WSL2. For this Windows-specific C++ project, you'll need to:
1. Work primarily on the Windows side (not in WSL)
2. Install Windows development tools
3. Use Windows Command Prompt or PowerShell for build commands

---

## 🛠️ Step-by-Step Setup

### Step 1: Install Visual Studio 2022

#### Option A: Visual Studio Community (Free)
1. **Download:** Go to https://visualstudio.microsoft.com/downloads/
2. **Select:** Visual Studio Community 2022 (Free)
3. **Run installer** and select these workloads:

```
☑️ Desktop development with C++
   ├── MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)
   ├── Windows 11 SDK (10.0.22000.0) or latest
   ├── CMake tools for Visual Studio
   ├── Git for Windows
   └── IntelliCode
```

#### Option B: Visual Studio Professional/Enterprise
If you have a license, follow the same process with additional features available.

### Step 2: Verify Installation

Open **Windows Command Prompt** (not WSL) and run:

```cmd
# Check Visual Studio compiler
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl

# Check CMake
cmake --version

# Check Git
git --version
```

### Step 3: Set Up vcpkg Package Manager

```cmd
# Clone vcpkg (in Windows, not WSL)
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Integrate with Visual Studio
.\vcpkg integrate install
```

### Step 4: Install Project Dependencies

```cmd
# Navigate to vcpkg directory
cd C:\vcpkg

# Install nlohmann/json library
.\vcpkg install nlohmann-json:x64-windows

# Install other dependencies we'll need
.\vcpkg install gtest:x64-windows
.\vcpkg install curl:x64-windows
```

### Step 5: Configure Project Build

Navigate to your project directory **in Windows** (not WSL):

```cmd
# Use Windows path, not /mnt/c/...
cd C:\workspace\transcriptor\TeamsTranscriptionApp

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

# Build the project
cmake --build . --config Debug
```

---

## 🔧 Current Setup Status

### ✅ Completed
- [x] Project repository created
- [x] Initial code structure in place
- [x] Documentation created

### 🔄 In Progress
- [ ] **Task 1.1:** Visual Studio 2022 installation
  - [ ] Download and install VS2022
  - [ ] Configure C++ workload
  - [ ] Verify compiler works
  - [ ] Test basic compilation

### ⏳ Pending
- [ ] **Task 1.2:** Dependency management setup
- [ ] **Task 1.3:** Build system validation

---

## 🚨 Important Notes

### Working Directory
- **WSL Path:** `/mnt/c/workspace/transcriptor/TeamsTranscriptionApp`
- **Windows Path:** `C:\workspace\transcriptor\TeamsTranscriptionApp`
- **Use Windows path** for all development tools

### File System Considerations
- Files created in WSL are accessible from Windows
- Build artifacts should be created on Windows side
- Git operations can be done from either side

### Common Issues and Solutions

#### Issue: "cl is not recognized"
**Solution:** Run the Visual Studio Developer Command Prompt or:
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

#### Issue: CMake not found
**Solution:** Add CMake to PATH or use Visual Studio's built-in CMake

#### Issue: vcpkg integration fails
**Solution:** Run as Administrator:
```cmd
.\vcpkg integrate install
```

---

## 🎯 Next Steps After Setup

1. **Validate build system** - Ensure project compiles
2. **Replace stub dependencies** - Update nlohmann/json
3. **Test basic functionality** - Verify audio APIs work
4. **Update progress tracking** - Mark tasks complete

---

## 🆘 Need Help?

### If Visual Studio Installation Fails:
1. Check available disk space (need ~3GB)
2. Run installer as Administrator
3. Disable antivirus temporarily
4. Check Windows Update status

### If Build Fails:
1. Verify all paths are correct
2. Check that Windows SDK is installed
3. Ensure vcpkg integration worked
4. Try building a simple "Hello World" first

### Quick Test Project
Create a simple test to verify everything works:

```cpp
// test.cpp
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    std::cout << "Hello, Windows C++ Development!" << std::endl;
    
    nlohmann::json j;
    j["test"] = "success";
    std::cout << "JSON library works: " << j.dump() << std::endl;
    
    return 0;
}
```

Compile with:
```cmd
cl test.cpp /I C:\vcpkg\installed\x64-windows\include /link C:\vcpkg\installed\x64-windows\lib\manual-link\nlohmann_json.lib
```

---

## 📞 Support Resources

- **Visual Studio Docs:** https://docs.microsoft.com/en-us/visualstudio/
- **CMake Documentation:** https://cmake.org/documentation/
- **vcpkg GitHub:** https://github.com/Microsoft/vcpkg
- **Windows SDK Docs:** https://docs.microsoft.com/en-us/windows/win32/

---

*Update PROGRESS.md with your progress as you complete each step!*