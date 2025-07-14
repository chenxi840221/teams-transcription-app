#!/usr/bin/env python3
"""
Project verification script for Teams Transcription Application
Checks if all required files are in place and validates basic structure
"""

import os
import sys

def check_file_exists(path, description):
    """Check if a file exists and report"""
    if os.path.exists(path):
        print(f"✓ {description}: {path}")
        return True
    else:
        print(f"✗ {description}: {path} (MISSING)")
        return False

def main():
    """Main verification function"""
    print("Teams Transcription Application - Project Verification")
    print("=" * 50)
    
    required_files = [
        ("CMakeLists.txt", "Main CMake configuration"),
        ("src/main.cpp", "Application entry point"),
        ("src/AudioCapture.h", "Audio capture header"),
        ("src/AudioCapture.cpp", "Audio capture implementation"),
        ("src/MainWindow.h", "Main window header"),
        ("src/MainWindow.cpp", "Main window implementation"),
        ("src/ConfigManager.h", "Configuration manager header"),
        ("src/ConfigManager.cpp", "Configuration manager implementation"),
        ("src/SpeechRecognition.h", "Speech recognition header"),
        ("src/SpeechRecognition.cpp", "Speech recognition implementation"),
        ("src/ProcessMonitor.h", "Process monitor header"),
        ("src/ProcessMonitor.cpp", "Process monitor implementation"),
        ("src/resource.h", "Resource definitions"),
        ("libs/json/include/nlohmann/json.hpp", "JSON library"),
        ("config/settings.example.json", "Example configuration"),
        ("resources/app.rc", "Windows resource file"),
        ("scripts/build.bat", "Build script"),
    ]
    
    all_good = True
    for path, description in required_files:
        if not check_file_exists(path, description):
            all_good = False
    
    print("\n" + "=" * 50)
    
    if all_good:
        print("✓ All required files are present!")
        print("\nNext steps:")
        print("1. Open project in Visual Studio on Windows")
        print("2. Configure API keys in config/settings.json")
        print("3. Build using scripts/build.bat or Visual Studio")
        print("4. Test audio capture and transcription features")
        return 0
    else:
        print("✗ Some required files are missing!")
        print("Please ensure all files are in place before building.")
        return 1

if __name__ == "__main__":
    sys.exit(main())