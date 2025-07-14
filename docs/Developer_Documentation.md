# Teams Audio Transcription Application
## Developer Documentation

**Version:** 2.0.0  
**Date:** July 2025  
**Document Type:** Technical Development Guide  

---

## Table of Contents

1. [Development Environment Setup](#1-development-environment-setup)
2. [Project Structure](#2-project-structure)
3. [Build System](#3-build-system)
4. [Code Standards](#4-code-standards)
5. [Component Development](#5-component-development)
6. [Testing Framework](#6-testing-framework)
7. [Debugging and Diagnostics](#7-debugging-and-diagnostics)
8. [Contributing Guidelines](#8-contributing-guidelines)

---

## 1. Development Environment Setup

### 1.1 Required Tools

#### Core Development Tools
- **Visual Studio 2019/2022**: Community, Professional, or Enterprise edition
- **CMake 3.20+**: Build system generator
- **Git**: Version control system
- **Windows SDK**: 10.0.19041.0 or later

#### Optional but Recommended
- **Visual Studio Code**: For lightweight editing and debugging
- **vcpkg**: C++ package manager for dependencies
- **Doxygen**: Documentation generation
- **ClangFormat**: Code formatting tool

### 1.2 Environment Configuration

#### Visual Studio Setup
1. **Install Visual Studio with the following workloads**:
   - Desktop development with C++
   - Windows 10/11 SDK (latest version)
   - CMake tools for Visual Studio
   - Git for Windows

2. **Configure C++ Development**:
   ```
   Individual components:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10/11 SDK (10.0.19041.0)
   - CMake tools for Visual Studio
   - Git for Windows
   - IntelliCode
   ```

#### CMake Configuration
```bash
# Install CMake (if not included with Visual Studio)
winget install Kitware.CMake

# Verify installation
cmake --version  # Should show 3.20+
```

#### Git Configuration
```bash
# Configure Git for development
git config --global user.name "Your Name"
git config --global user.email "your.email@company.com"
git config --global core.autocrlf true
git config --global core.editor "code --wait"
```

### 1.3 Repository Setup

#### Clone the Repository
```bash
# Clone the main repository
git clone https://github.com/yourorg/teams-transcription-app.git
cd teams-transcription-app

# Initialize submodules (if any)
git submodule update --init --recursive
```

#### Environment Variables
Create a `.env` file in the project root:
```bash
# Speech Recognition API Keys (for testing)
AZURE_SPEECH_KEY=your_azure_key_here
AZURE_SPEECH_REGION=eastus
GOOGLE_SPEECH_KEY=your_google_key_here
OPENAI_API_KEY=your_openai_key_here

# Build Configuration
CMAKE_BUILD_TYPE=Debug
CMAKE_GENERATOR="Visual Studio 17 2022"
```

### 1.4 Dependencies Management

#### External Dependencies
The project uses the following external libraries:

```cmake
# nlohmann/json (included)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(libs/json)

# Windows SDK (system)
find_package(WindowsSDK REQUIRED)

# Threading support
find_package(Threads REQUIRED)
```

#### Speech Recognition SDKs
For full functionality, install speech recognition SDKs:

**Azure Cognitive Services SDK**:
```bash
# Using vcpkg
vcpkg install microsoft-cognitiveservices-speech-cpp:x64-windows
```

**Google Cloud Speech SDK**:
```bash
# Using vcpkg  
vcpkg install google-cloud-cpp[speech]:x64-windows
```

---

## 2. Project Structure

### 2.1 Directory Organization

```
TeamsTranscriptionApp/
├── src/                     # Source code
│   ├── main.cpp            # Application entry point
│   ├── AudioCapture.h/.cpp # Audio capture implementation
│   ├── MainWindow.h/.cpp   # GUI implementation
│   ├── SpeechRecognition.h/.cpp # Speech recognition
│   ├── ProcessMonitor.h/.cpp    # Teams process monitoring
│   ├── ConfigManager.h/.cpp     # Configuration management
│   └── resource.h          # Resource definitions
├── include/                # Public headers
├── libs/                   # Third-party libraries
│   ├── json/              # nlohmann/json
│   ├── httplib/           # HTTP client library
│   └── speech/            # Speech recognition SDKs
├── resources/             # Application resources
│   ├── app.rc             # Windows resource file
│   ├── icons/             # Application icons
│   ├── sounds/            # Audio files
│   └── manifest.xml       # Application manifest
├── config/                # Configuration files
│   ├── settings.json      # Default settings
│   └── settings.example.json # Example configuration
├── data/                  # Runtime data (created at runtime)
│   ├── recordings/        # Audio recordings
│   ├── transcripts/       # Transcript files
│   └── exports/           # Exported documents
├── docs/                  # Documentation
├── tests/                 # Test suite
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── scripts/               # Build and utility scripts
│   ├── build.bat          # Windows build script
│   ├── clean.bat          # Cleanup script
│   └── setup.bat          # Environment setup
├── tools/                 # Development tools
├── CMakeLists.txt         # Main CMake configuration
├── .gitignore             # Git ignore rules
├── README.md              # Project overview
└── LICENSE                # License file
```

### 2.2 Source Code Organization

#### Core Components
- **AudioCapture**: WASAPI-based audio capture
- **SpeechRecognition**: Multi-provider speech recognition
- **MainWindow**: Win32 GUI implementation
- **ProcessMonitor**: Teams process detection
- **ConfigManager**: JSON configuration management

#### Support Components
- **Export**: Document export functionality
- **Logging**: Application logging system
- **Security**: Encryption and privacy features
- **Utils**: Common utility functions

### 2.3 Header Dependencies

```cpp
// Typical include order
#include "AudioCapture.h"      // Core audio functionality
#include "SpeechRecognition.h" // Speech recognition
#include "ProcessMonitor.h"    // Teams integration
#include "ConfigManager.h"     // Configuration
#include "MainWindow.h"        // GUI (includes others)
```

---

## 3. Build System

### 3.1 CMake Configuration

#### Main CMakeLists.txt Overview
```cmake
cmake_minimum_required(VERSION 3.20)
project(TeamsTranscriptionApp VERSION 2.0.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Platform-specific settings
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DUNICODE -D_UNICODE)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# Find dependencies
find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/json/include
)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h" "include/*.h")

# Resource files
if(WIN32)
    set(RESOURCES "resources/app.rc")
endif()

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${RESOURCES})

# Link libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        kernel32 user32 gdi32 winspool comdlg32 advapi32 shell32
        ole32 oleaut32 uuid odbc32 odbccp32 winmm mmdevapi
        comctl32 shlwapi
    )
endif()

# Copy configuration files
configure_file(config/settings.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/settings.json COPYONLY)
```

#### Build Configurations
```cmake
# Debug configuration
set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Od /Zi /EHsc /W4")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")

# Release configuration  
set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /DNDEBUG /EHsc /W3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE")

# Enable static analysis in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
endif()
```

### 3.2 Build Scripts

#### Windows Build Script (scripts/build.bat)
```batch
@echo off
setlocal EnableDelayedExpansion

echo Building Teams Audio Transcription Application...

REM Check for CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found in PATH
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%

if %errorlevel% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo Build completed successfully!
echo Executable: build\output\Release\TeamsTranscriptionApp.exe
```

#### Development Build Script (scripts/build-dev.bat)
```batch
@echo off
echo Building DEBUG configuration...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug

echo Debug build completed!
echo Executable: build\output\Debug\TeamsTranscriptionApp.exe
```

### 3.3 Build Targets

#### Available Build Targets
```bash
# Build main application
cmake --build . --target TeamsTranscriptionApp

# Build tests
cmake --build . --target tests

# Build documentation
cmake --build . --target docs

# Build installer package
cmake --build . --target package

# Clean build artifacts
cmake --build . --target clean
```

#### Custom Build Targets
```cmake
# Add custom target for code formatting
add_custom_target(format
    COMMAND clang-format -i ${SOURCES} ${HEADERS}
    COMMENT "Formatting source code"
)

# Add custom target for static analysis
add_custom_target(analyze
    COMMAND "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC/14.30.30705/bin/Hostx64/x64/cl.exe" /analyze ${SOURCES}
    COMMENT "Running static analysis"
)

# Add custom target for documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation"
    )
endif()
```

---

## 4. Code Standards

### 4.1 C++ Style Guidelines

#### Naming Conventions
```cpp
// Classes: PascalCase
class AudioCapture {
public:
    // Public methods: PascalCase
    HRESULT StartCapture();
    void StopCapture();
    
    // Public members: camelCase (prefer private with accessors)
    bool isCapturing;

private:
    // Private members: camelCase with prefix
    IMMDeviceEnumerator* deviceEnumerator;
    std::atomic<bool> isRecording;
    
    // Private methods: PascalCase
    void ProcessAudioData();
    HRESULT InitializeWASAPI();
};

// Constants: UPPER_SNAKE_CASE
const int MAX_BUFFER_SIZE = 4096;
const wchar_t* WINDOW_CLASS_NAME = L"TeamsTranscriptionWindow";

// Enums: PascalCase with PascalCase values
enum class AudioQuality {
    Low = 8000,
    Medium = 16000,
    High = 44100
};

// Variables: camelCase
int sampleRate = 16000;
std::string configPath = "config/settings.json";

// Functions: PascalCase
bool LoadConfiguration(const std::string& path);
HRESULT InitializeAudioDevice();
```

#### File Organization
```cpp
// AudioCapture.h
#pragma once

// System includes first
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

// Standard library includes
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

// Project includes last
#include "Types.h"

class AudioCapture {
    // Public section first
public:
    // Type definitions
    enum class AudioQuality { /* ... */ };
    struct AudioFormat { /* ... */ };
    using AudioDataCallback = std::function<void(const std::vector<BYTE>&, const AudioFormat&)>;
    
    // Constructors/destructors
    AudioCapture();
    ~AudioCapture();
    
    // Main interface methods
    HRESULT Initialize(AudioQuality quality = AudioQuality::Medium);
    HRESULT StartCapture();
    void StopCapture();
    
    // Accessors
    bool IsCapturing() const { return isCapturing.load(); }
    AudioFormat GetAudioFormat() const;
    
    // Configuration
    void SetAudioDataCallback(AudioDataCallback callback);

// Private section last
private:
    // Member variables
    IMMDeviceEnumerator* deviceEnumerator;
    IAudioClient* audioClient;
    std::atomic<bool> isCapturing;
    
    // Private methods
    void CaptureThreadProc();
    HRESULT InitializeWASAPI();
    void Cleanup();
};
```

#### Error Handling Patterns
```cpp
// Use HRESULT for Windows API calls
HRESULT AudioCapture::Initialize() {
    HRESULT hr = S_OK;
    
    hr = CoCreateInstance(/* ... */);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator: 0x" << std::hex << hr << std::endl;
        return hr;
    }
    
    // Continue with other operations...
    return S_OK;
}

// Use exceptions for internal errors
void ConfigManager::LoadConfig(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Config path cannot be empty");
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + path);
    }
    
    // Process file...
}

// Use optional for nullable returns
std::optional<AudioFormat> AudioCapture::GetCurrentFormat() const {
    if (!isInitialized) {
        return std::nullopt;
    }
    return currentFormat;
}
```

### 4.2 Documentation Standards

#### Header Documentation
```cpp
/**
 * @file AudioCapture.h
 * @brief Audio capture functionality using Windows WASAPI
 * @author Development Team
 * @date 2025-07-14
 * @version 2.0.0
 */

/**
 * @class AudioCapture
 * @brief Provides real-time audio capture from system output using WASAPI loopback
 * 
 * This class implements Windows Audio Session API (WASAPI) loopback capture to
 * record system audio output. It supports various audio qualities and provides
 * real-time audio data through a callback mechanism.
 * 
 * @example
 * @code
 * AudioCapture capture;
 * if (SUCCEEDED(capture.Initialize(AudioCapture::AudioQuality::Medium))) {
 *     capture.SetAudioDataCallback([](const auto& data, const auto& format) {
 *         // Process audio data
 *     });
 *     capture.StartCapture();
 * }
 * @endcode
 * 
 * @see https://docs.microsoft.com/en-us/windows/win32/coreaudio/wasapi
 */
class AudioCapture {
public:
    /**
     * @brief Audio quality enumeration
     * 
     * Defines standard audio quality levels with corresponding sample rates.
     * Higher quality requires more processing power and storage.
     */
    enum class AudioQuality {
        Low = 8000,     ///< 8kHz - Voice quality, minimal resources
        Medium = 16000, ///< 16kHz - Standard quality, recommended
        High = 44100    ///< 44.1kHz - CD quality, maximum resources
    };
    
    /**
     * @brief Initialize the audio capture system
     * @param quality Desired audio quality level
     * @return S_OK on success, error HRESULT on failure
     * 
     * This method initializes the WASAPI system and prepares for audio capture.
     * It must be called before StartCapture().
     * 
     * @note Requires COM to be initialized before calling
     */
    HRESULT Initialize(AudioQuality quality = AudioQuality::Medium);
};
```

#### Implementation Documentation
```cpp
// AudioCapture.cpp

HRESULT AudioCapture::Initialize(AudioQuality quality) {
    HRESULT hr = S_OK;
    
    // Initialize COM device enumerator for audio devices
    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
        IID_IMMDeviceEnumerator, (void**)&deviceEnumerator);
    
    if (FAILED(hr)) {
        // Log error with context for debugging
        std::cerr << "Failed to create device enumerator: 0x" << std::hex << hr << std::endl;
        return hr;
    }
    
    // Get default render device for loopback capture
    // We use render endpoint because we want to capture what's being played
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint: 0x" << std::hex << hr << std::endl;
        return hr;
    }
    
    // Continue initialization...
    return InitializeAudioClient(quality);
}
```

### 4.3 Code Quality Standards

#### Static Analysis
```cpp
// Enable all warnings and treat as errors in development
#pragma warning(push)
#pragma warning(error: 4996) // Deprecated function usage
#pragma warning(error: 4311) // Pointer truncation
#pragma warning(error: 4302) // Type cast truncation

// Disable specific warnings for third-party code
#pragma warning(disable: 4100) // Unreferenced formal parameter
#include "third_party_header.h"
#pragma warning(pop)
```

#### Memory Management
```cpp
// Use RAII for Windows resources
class AudioCaptureRAII {
private:
    IMMDeviceEnumerator* deviceEnumerator_;
    IAudioClient* audioClient_;
    
public:
    AudioCaptureRAII() : deviceEnumerator_(nullptr), audioClient_(nullptr) {}
    
    ~AudioCaptureRAII() {
        SafeRelease(&audioClient_);
        SafeRelease(&deviceEnumerator_);
    }
    
    template<class T>
    void SafeRelease(T** ppT) {
        if (*ppT) {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }
};

// Use smart pointers for modern C++
class MainWindow {
private:
    std::unique_ptr<AudioCapture> audioCapture_;
    std::shared_ptr<ConfigManager> configManager_;
    std::weak_ptr<ExportManager> exportManager_;
};
```

---

## 5. Component Development

### 5.1 Adding New Speech Providers

#### Step 1: Define Provider Interface
```cpp
// In SpeechRecognition.h
enum class Provider {
    Azure,
    Google,
    OpenAI,
    Amazon,
    Windows,
    Custom      // Add new provider type
};

// Add configuration structure
struct CustomProviderConfig {
    std::string endpoint;
    std::string apiKey;
    std::string model;
    int maxRetries;
};
```

#### Step 2: Implement Provider Class
```cpp
// CustomSpeechProvider.h
#pragma once
#include "SpeechRecognition.h"

class CustomSpeechProvider : public SpeechRecognition::ISpeechProvider {
public:
    CustomSpeechProvider();
    ~CustomSpeechProvider() override;
    
    bool Initialize(const SpeechRecognition::SpeechConfig& config) override;
    void ProcessAudioData(const std::vector<BYTE>& audioData, 
                         const AudioCapture::AudioFormat& format) override;
    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback callback) override;
    bool IsInitialized() const override;

private:
    bool initialized_;
    SpeechRecognition::SpeechConfig config_;
    SpeechRecognition::TranscriptionCallback callback_;
    
    // Provider-specific members
    std::string endpoint_;
    HttpClient httpClient_;
    AudioBuffer audioBuffer_;
    
    // Private methods
    bool ConnectToService();
    std::string ConvertAudioToFormat(const std::vector<BYTE>& audioData,
                                   const AudioCapture::AudioFormat& format);
    bool SendAudioChunk(const std::string& audioData);
    void ProcessResponse(const std::string& response);
};
```

#### Step 3: Implement Provider Logic
```cpp
// CustomSpeechProvider.cpp
#include "CustomSpeechProvider.h"
#include <json/json.h>

bool CustomSpeechProvider::Initialize(const SpeechRecognition::SpeechConfig& config) {
    config_ = config;
    endpoint_ = "https://api.customspeech.com/v1/recognize";
    
    // Validate configuration
    if (config_.apiKey.empty()) {
        std::cerr << "Custom speech provider requires API key" << std::endl;
        return false;
    }
    
    // Initialize HTTP client
    httpClient_.SetHeader("Authorization", "Bearer " + config_.apiKey);
    httpClient_.SetHeader("Content-Type", "application/json");
    
    // Test connection
    if (!ConnectToService()) {
        std::cerr << "Failed to connect to custom speech service" << std::endl;
        return false;
    }
    
    initialized_ = true;
    return true;
}

void CustomSpeechProvider::ProcessAudioData(const std::vector<BYTE>& audioData,
                                           const AudioCapture::AudioFormat& format) {
    if (!initialized_ || !callback_) {
        return;
    }
    
    // Buffer audio data
    audioBuffer_.Append(audioData);
    
    // Process when we have enough data (e.g., 3 seconds)
    if (audioBuffer_.GetDurationMs() >= 3000) {
        std::string audioBase64 = audioBuffer_.ToBase64();
        
        // Create request payload
        nlohmann::json request;
        request["audio"] = audioBase64;
        request["format"] = {
            {"sample_rate", format.sampleRate},
            {"channels", format.channels},
            {"bits_per_sample", format.bitsPerSample}
        };
        request["language"] = config_.language;
        request["enable_punctuation"] = config_.enablePunctuation;
        
        // Send to API
        auto response = httpClient_.Post(endpoint_, request.dump());
        if (response.status == 200) {
            ProcessResponse(response.body);
        }
        
        audioBuffer_.Clear();
    }
}

void CustomSpeechProvider::ProcessResponse(const std::string& response) {
    try {
        nlohmann::json json = nlohmann::json::parse(response);
        
        if (json.contains("transcription")) {
            std::string text = json["transcription"];
            double confidence = json.value("confidence", 0.0);
            
            if (callback_ && !text.empty()) {
                callback_(text, confidence);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing speech response: " << e.what() << std::endl;
    }
}
```

#### Step 4: Register Provider
```cpp
// In SpeechRecognition.cpp
bool SpeechRecognition::InitializeProvider() {
    speechProvider_.reset();
    
    try {
        switch (currentConfig_.provider) {
            case Provider::Azure:
                speechProvider_ = std::make_unique<AzureSpeechProvider>();
                break;
                
            case Provider::Google:
                speechProvider_ = std::make_unique<GoogleSpeechProvider>();
                break;
                
            case Provider::Custom:  // Add new case
                speechProvider_ = std::make_unique<CustomSpeechProvider>();
                break;
                
            default:
                std::cerr << "Unknown speech provider" << std::endl;
                return false;
        }
        
        // Rest of initialization...
    } catch (const std::exception& e) {
        std::cerr << "Exception initializing speech provider: " << e.what() << std::endl;
        return false;
    }
}
```

### 5.2 Adding Export Formats

#### Step 1: Define Export Interface
```cpp
// ExportManager.h
#pragma once
#include "TranscriptData.h"

class IExportFormat {
public:
    virtual ~IExportFormat() = default;
    virtual bool Export(const MeetingTranscript& transcript, 
                       const std::string& outputPath) = 0;
    virtual std::string GetFileExtension() const = 0;
    virtual std::string GetFormatName() const = 0;
    virtual bool SupportsTemplate() const = 0;
};

class ExportManager {
public:
    enum class Format {
        TXT,
        DOCX,
        PDF,
        JSON,
        XML,     // Add new format
        Markdown // Add new format
    };
    
    bool RegisterFormat(Format format, std::unique_ptr<IExportFormat> exporter);
    bool Export(const MeetingTranscript& transcript, Format format, 
               const std::string& outputPath, const std::string& templatePath = "");
    std::vector<Format> GetSupportedFormats() const;

private:
    std::map<Format, std::unique_ptr<IExportFormat>> exporters_;
};
```

#### Step 2: Implement Format Handler
```cpp
// MarkdownExporter.h
#pragma once
#include "ExportManager.h"

class MarkdownExporter : public IExportFormat {
public:
    bool Export(const MeetingTranscript& transcript, 
               const std::string& outputPath) override;
    std::string GetFileExtension() const override { return ".md"; }
    std::string GetFormatName() const override { return "Markdown"; }
    bool SupportsTemplate() const override { return true; }

private:
    std::string FormatHeader(const MeetingTranscript& transcript);
    std::string FormatSegment(const TranscriptionSegment& segment);
    std::string FormatFooter(const MeetingTranscript& transcript);
    std::string EscapeMarkdown(const std::string& text);
};

// MarkdownExporter.cpp
bool MarkdownExporter::Export(const MeetingTranscript& transcript, 
                             const std::string& outputPath) {
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open output file: " << outputPath << std::endl;
        return false;
    }
    
    // Write header
    file << FormatHeader(transcript) << "\n\n";
    
    // Write transcript segments
    for (const auto& segment : transcript.segments) {
        file << FormatSegment(segment) << "\n\n";
    }
    
    // Write footer
    file << FormatFooter(transcript);
    
    file.close();
    return true;
}

std::string MarkdownExporter::FormatHeader(const MeetingTranscript& transcript) {
    std::ostringstream oss;
    oss << "# Meeting Transcript\n\n";
    oss << "**Meeting ID:** " << transcript.meetingId << "\n";
    oss << "**Start Time:** " << FormatTime(transcript.startTime) << "\n";
    oss << "**End Time:** " << FormatTime(transcript.endTime) << "\n";
    oss << "**Duration:** " << CalculateDuration(transcript) << "\n";
    oss << "**Speech Provider:** " << transcript.metadata.speechProvider << "\n";
    return oss.str();
}

std::string MarkdownExporter::FormatSegment(const TranscriptionSegment& segment) {
    std::ostringstream oss;
    oss << "**[" << FormatTime(segment.startTime) << "]** ";
    
    if (!segment.speakerId.empty() && segment.speakerId != "unknown") {
        oss << "**" << segment.speakerId << ":** ";
    }
    
    oss << EscapeMarkdown(segment.text);
    
    if (segment.confidence < 0.8) {
        oss << " *(low confidence)*";
    }
    
    return oss.str();
}
```

### 5.3 Adding UI Components

#### Step 1: Define Component Interface
```cpp
// UIComponent.h
#pragma once
#include <windows.h>

class UIComponent {
public:
    virtual ~UIComponent() = default;
    virtual bool Create(HWND parent, int x, int y, int width, int height) = 0;
    virtual void Update() = 0;
    virtual void Show(bool visible) = 0;
    virtual HWND GetHandle() const = 0;
    virtual void OnCommand(WORD commandId, WORD notificationCode) {}
    virtual void OnPaint(HDC hdc) {}
    virtual void OnResize(int width, int height) {}

protected:
    HWND hwnd_ = nullptr;
    HWND parent_ = nullptr;
    int x_ = 0, y_ = 0, width_ = 0, height_ = 0;
};
```

#### Step 2: Implement Custom Control
```cpp
// TranscriptionView.h
#pragma once
#include "UIComponent.h"
#include "TranscriptData.h"
#include <vector>

class TranscriptionView : public UIComponent {
public:
    TranscriptionView();
    ~TranscriptionView() override;
    
    bool Create(HWND parent, int x, int y, int width, int height) override;
    void Update() override;
    void Show(bool visible) override;
    HWND GetHandle() const override { return hwnd_; }
    
    // Transcription-specific methods
    void AddSegment(const TranscriptionSegment& segment);
    void Clear();
    void SetAutoScroll(bool enabled);
    void SetFontSize(int size);
    void SetShowTimestamps(bool show);
    void SetShowConfidence(bool show);
    
    // Event handlers
    void OnPaint(HDC hdc) override;
    void OnResize(int width, int height) override;

private:
    std::vector<TranscriptionSegment> segments_;
    bool autoScroll_ = true;
    bool showTimestamps_ = true;
    bool showConfidence_ = false;
    int fontSize_ = 12;
    int scrollPos_ = 0;
    
    HFONT font_ = nullptr;
    HBRUSH backgroundBrush_ = nullptr;
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void RegisterWindowClass();
    void UpdateScrollbar();
    void DrawSegment(HDC hdc, const TranscriptionSegment& segment, RECT& rect);
    int CalculateSegmentHeight(HDC hdc, const TranscriptionSegment& segment, int width);
};
```

---

## 6. Testing Framework

### 6.1 Unit Testing Setup

#### Test Framework Configuration
```cmake
# CMakeLists.txt - Testing section
option(BUILD_TESTS "Build test suite" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Google Test framework
    find_package(GTest CONFIG REQUIRED)
    
    # Test executable
    file(GLOB_RECURSE TEST_SOURCES "tests/unit/*.cpp")
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests 
        GTest::gtest 
        GTest::gtest_main
        TeamsTranscriptionLib  # Static library with core code
    )
    
    # Register tests with CTest
    add_test(NAME UnitTests COMMAND unit_tests)
    
    # Coverage target
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(unit_tests PRIVATE --coverage)
        target_link_options(unit_tests PRIVATE --coverage)
    endif()
endif()
```

#### Test Structure
```cpp
// tests/unit/AudioCaptureTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AudioCapture.h"

class AudioCaptureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize COM for tests
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        audioCapture = std::make_unique<AudioCapture>();
    }
    
    void TearDown() override {
        audioCapture.reset();
        CoUninitialize();
    }
    
    std::unique_ptr<AudioCapture> audioCapture;
};

TEST_F(AudioCaptureTest, InitializeWithValidParameters) {
    HRESULT hr = audioCapture->Initialize(AudioCapture::AudioQuality::Medium);
    EXPECT_TRUE(SUCCEEDED(hr));
    EXPECT_FALSE(audioCapture->IsCapturing());
}

TEST_F(AudioCaptureTest, StartCaptureAfterInitialization) {
    audioCapture->Initialize(AudioCapture::AudioQuality::Low);
    
    HRESULT hr = audioCapture->StartCapture();
    EXPECT_TRUE(SUCCEEDED(hr));
    EXPECT_TRUE(audioCapture->IsCapturing());
    
    audioCapture->StopCapture();
    EXPECT_FALSE(audioCapture->IsCapturing());
}

TEST_F(AudioCaptureTest, CallbackReceivesAudioData) {
    bool callbackCalled = false;
    std::vector<BYTE> receivedData;
    AudioCapture::AudioFormat receivedFormat;
    
    audioCapture->SetAudioDataCallback([&](const std::vector<BYTE>& data, 
                                           const AudioCapture::AudioFormat& format) {
        callbackCalled = true;
        receivedData = data;
        receivedFormat = format;
    });
    
    audioCapture->Initialize();
    audioCapture->StartCapture();
    
    // Wait for callback (in real test, use proper synchronization)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    audioCapture->StopCapture();
    
    // Verify callback was called with valid data
    EXPECT_TRUE(callbackCalled);
    EXPECT_GT(receivedData.size(), 0);
    EXPECT_GT(receivedFormat.sampleRate, 0);
}
```

### 6.2 Integration Testing

#### Test Application Framework
```cpp
// tests/integration/ApplicationTest.cpp
#include <gtest/gtest.h>
#include "MainWindow.h"
#include "AudioCapture.h"
#include "SpeechRecognition.h"

class ApplicationIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        CreateTestConfiguration();
        
        // Initialize application components
        mainWindow = std::make_unique<MainWindow>();
        ASSERT_TRUE(mainWindow->Create(GetModuleHandle(nullptr), SW_HIDE));
    }
    
    void TearDown() override {
        mainWindow.reset();
        CleanupTestConfiguration();
    }
    
    void CreateTestConfiguration() {
        // Create minimal config for testing
        nlohmann::json config;
        config["speechRecognition"]["provider"] = "windows";
        config["recording"]["audioQuality"] = 16000;
        
        std::ofstream file("test_settings.json");
        file << config.dump(4);
        file.close();
    }
    
    std::unique_ptr<MainWindow> mainWindow;
};

TEST_F(ApplicationIntegrationTest, FullRecordingWorkflow) {
    // Start recording
    mainWindow->StartRecording();
    EXPECT_TRUE(mainWindow->IsRecording());
    
    // Simulate some audio processing time
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Stop recording
    mainWindow->StopRecording();
    EXPECT_FALSE(mainWindow->IsRecording());
    
    // Verify transcript was created
    auto transcript = mainWindow->GetCurrentTranscript();
    EXPECT_FALSE(transcript.segments.empty());
}

TEST_F(ApplicationIntegrationTest, ConfigurationPersistence) {
    // Modify configuration
    auto& config = mainWindow->GetConfigManager().GetConfig();
    config.audioQuality = 44100;
    config.speechConfig.language = "en-GB";
    
    // Save configuration
    EXPECT_TRUE(mainWindow->GetConfigManager().SaveConfig("test_config.json"));
    
    // Create new config manager and load
    ConfigManager newConfigManager;
    EXPECT_TRUE(newConfigManager.LoadConfig("test_config.json"));
    
    // Verify settings were persisted
    const auto& loadedConfig = newConfigManager.GetConfig();
    EXPECT_EQ(loadedConfig.audioQuality, 44100);
    EXPECT_EQ(loadedConfig.speechConfig.language, "en-GB");
}
```

### 6.3 Performance Testing

#### Benchmark Framework
```cpp
// tests/performance/AudioCaptureBenchmark.cpp
#include <benchmark/benchmark.h>
#include "AudioCapture.h"

static void BM_AudioCaptureInitialization(benchmark::State& state) {
    for (auto _ : state) {
        AudioCapture capture;
        benchmark::DoNotOptimize(capture.Initialize());
        capture.Cleanup();
    }
}
BENCHMARK(BM_AudioCaptureInitialization);

static void BM_AudioDataProcessing(benchmark::State& state) {
    AudioCapture capture;
    capture.Initialize();
    
    // Create test audio data
    std::vector<BYTE> testData(state.range(0));
    std::fill(testData.begin(), testData.end(), 127);
    
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(capture.ProcessAudioData(testData.data(), testData.size(), 0));
    }
    
    state.SetBytesProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_AudioDataProcessing)->Range(1024, 1024*64);

static void BM_SpeechRecognitionLatency(benchmark::State& state) {
    SpeechRecognition speech;
    SpeechRecognition::SpeechConfig config;
    config.provider = SpeechRecognition::Provider::Windows;
    speech.Initialize(config);
    
    std::vector<BYTE> audioData(16000 * 2); // 1 second of 16kHz audio
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        speech.ProcessAudioData(audioData, format);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        state.SetIterationTime(duration.count() / 1000000.0);
    }
}
BENCHMARK(BM_SpeechRecognitionLatency)->UseManualTime();

BENCHMARK_MAIN();
```

---

## 7. Debugging and Diagnostics

### 7.1 Debugging Setup

#### Visual Studio Configuration
```json
// .vscode/launch.json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Teams Transcription",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/output/Debug/TeamsTranscriptionApp.exe",
            "args": ["--debug", "--config=debug_settings.json"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "TEAMS_TRANSCRIPTION_DEBUG",
                    "value": "1"
                }
            ],
            "console": "externalTerminal"
        },
        {
            "name": "Attach to Process",
            "type": "cppvsdbg",
            "request": "attach",
            "processId": "${command:pickProcess}"
        }
    ]
}
```

#### Debug Build Configuration
```cmake
# Debug-specific settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_BUILD)
    add_definitions(-D_DEBUG)
    
    # Enable debug heap
    add_definitions(-D_CRTDBG_MAP_ALLOC)
    
    # Enable static analysis
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")
    
    # Enable address sanitizer (if available)
    if(MSVC_VERSION GREATER_EQUAL 1929)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    endif()
endif()
```

### 7.2 Logging Framework

#### Logger Implementation
```cpp
// Logger.h
#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <memory>

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5
};

class Logger {
public:
    static Logger& GetInstance();
    
    void SetLevel(LogLevel level);
    void SetOutputFile(const std::string& filename);
    void EnableConsoleOutput(bool enable);
    
    void Log(LogLevel level, const std::string& message, 
             const char* file = __FILE__, int line = __LINE__);
    
    template<typename... Args>
    void LogFormat(LogLevel level, const char* format, Args&&... args) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
        Log(level, std::string(buffer));
    }

private:
    Logger() = default;
    
    LogLevel currentLevel_ = LogLevel::Info;
    std::ofstream logFile_;
    bool consoleOutput_ = false;
    std::mutex mutex_;
    
    std::string FormatMessage(LogLevel level, const std::string& message,
                             const char* file, int line);
    const char* LevelToString(LogLevel level);
};

// Macros for convenient logging
#define LOG_TRACE(msg) Logger::GetInstance().Log(LogLevel::Trace, msg, __FILE__, __LINE__)
#define LOG_DEBUG(msg) Logger::GetInstance().Log(LogLevel::Debug, msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::GetInstance().Log(LogLevel::Info, msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::GetInstance().Log(LogLevel::Warning, msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::GetInstance().Log(LogLevel::Error, msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::GetInstance().Log(LogLevel::Critical, msg, __FILE__, __LINE__)

#define LOG_DEBUG_F(format, ...) Logger::GetInstance().LogFormat(LogLevel::Debug, format, __VA_ARGS__)
#define LOG_INFO_F(format, ...) Logger::GetInstance().LogFormat(LogLevel::Info, format, __VA_ARGS__)
#define LOG_ERROR_F(format, ...) Logger::GetInstance().LogFormat(LogLevel::Error, format, __VA_ARGS__)
```

#### Usage in Components
```cpp
// AudioCapture.cpp
HRESULT AudioCapture::Initialize(AudioQuality quality) {
    LOG_INFO("Initializing audio capture with quality: " + std::to_string((int)quality));
    
    HRESULT hr = CoCreateInstance(/* ... */);
    if (FAILED(hr)) {
        LOG_ERROR_F("Failed to create device enumerator: 0x%08X", hr);
        return hr;
    }
    
    LOG_DEBUG("Device enumerator created successfully");
    
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);
    if (FAILED(hr)) {
        LOG_ERROR_F("Failed to get default audio endpoint: 0x%08X", hr);
        return hr;
    }
    
    LOG_INFO("Audio capture initialized successfully");
    return S_OK;
}
```

### 7.3 Diagnostic Tools

#### Performance Monitor
```cpp
// PerformanceMonitor.h
#pragma once
#include <chrono>
#include <string>
#include <unordered_map>

class PerformanceMonitor {
public:
    static PerformanceMonitor& GetInstance();
    
    void StartTimer(const std::string& name);
    void StopTimer(const std::string& name);
    void RecordMetric(const std::string& name, double value);
    
    void PrintReport();
    void SaveReport(const std::string& filename);

private:
    struct TimerData {
        std::chrono::high_resolution_clock::time_point startTime;
        double totalTime = 0.0;
        int callCount = 0;
    };
    
    std::unordered_map<std::string, TimerData> timers_;
    std::unordered_map<std::string, std::vector<double>> metrics_;
    std::mutex mutex_;
};

// RAII timer for automatic measurement
class ScopedTimer {
public:
    ScopedTimer(const std::string& name) : name_(name) {
        PerformanceMonitor::GetInstance().StartTimer(name_);
    }
    
    ~ScopedTimer() {
        PerformanceMonitor::GetInstance().StopTimer(name_);
    }

private:
    std::string name_;
};

#define PERF_TIMER(name) ScopedTimer timer(name)
#define PERF_FUNCTION_TIMER() ScopedTimer timer(__FUNCTION__)
```

#### Memory Leak Detection
```cpp
// MemoryDiagnostics.h (Debug builds only)
#pragma once

#ifdef _DEBUG
#include <crtdbg.h>

class MemoryDiagnostics {
public:
    static void Initialize() {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    }
    
    static void SetBreakpoint(int allocNumber) {
        _CrtSetBreakAlloc(allocNumber);
    }
    
    static void DumpLeaks() {
        _CrtDumpMemoryLeaks();
    }
    
    static void CheckHeap() {
        if (!_CrtCheckMemory()) {
            LOG_ERROR("Heap corruption detected!");
        }
    }
};

#define MEMORY_INIT() MemoryDiagnostics::Initialize()
#define MEMORY_CHECK() MemoryDiagnostics::CheckHeap()
#define MEMORY_DUMP_LEAKS() MemoryDiagnostics::DumpLeaks()
#else
#define MEMORY_INIT()
#define MEMORY_CHECK()
#define MEMORY_DUMP_LEAKS()
#endif
```

---

## 8. Contributing Guidelines

### 8.1 Development Workflow

#### Branch Strategy
```
main                    # Production-ready code
├── develop            # Integration branch
├── feature/           # New features
│   ├── feature/azure-speech-v2
│   └── feature/export-templates
├── bugfix/           # Bug fixes
│   ├── bugfix/audio-capture-leak
│   └── bugfix/ui-scaling
├── hotfix/           # Critical production fixes
└── release/          # Release preparation
    └── release/v2.1.0
```

#### Pull Request Process
1. **Create Feature Branch**:
   ```bash
   git checkout develop
   git pull origin develop
   git checkout -b feature/new-export-format
   ```

2. **Implement Changes**:
   - Follow coding standards
   - Add unit tests
   - Update documentation
   - Test thoroughly

3. **Submit Pull Request**:
   - Target `develop` branch
   - Include description of changes
   - Reference related issues
   - Ensure CI passes

4. **Code Review**:
   - Address reviewer feedback
   - Update tests as needed
   - Squash commits if requested

5. **Merge**:
   - Use "Squash and merge" for features
   - Use "Merge commit" for releases

### 8.2 Code Review Guidelines

#### Review Checklist
- [ ] Code follows style guidelines
- [ ] Changes are well-documented
- [ ] Unit tests are included and pass
- [ ] No obvious performance issues
- [ ] Error handling is appropriate
- [ ] Security considerations addressed
- [ ] Breaking changes are documented
- [ ] Dependencies are justified

#### Review Comments Format
```
// Suggestion: Consider using std::make_unique here
std::unique_ptr<AudioCapture> capture(new AudioCapture());

// Question: Should this handle the case where config is null?
config->LoadSettings();

// Issue: This could cause a memory leak
IMMDevice* device = nullptr;
// Missing: device->Release()

// Praise: Nice use of RAII here!
std::lock_guard<std::mutex> lock(mutex_);
```

### 8.3 Documentation Standards

#### Code Documentation
- All public APIs must have doxygen comments
- Complex algorithms need explanatory comments
- Include usage examples for new features
- Document any threading considerations
- Explain Windows-specific code for maintainers

#### Commit Message Format
```
type(scope): subject

body

footer
```

Examples:
```
feat(audio): add support for 48kHz capture rate

- Implement higher quality audio capture option
- Update UI to include 48kHz selection
- Add performance tests for high-rate capture

Closes #123
```

```
fix(speech): resolve Azure API authentication timeout

The authentication was timing out due to incorrect region
configuration. Updated to use dynamic region detection.

Fixes #456
```

```
docs(api): update speech provider integration guide

- Add examples for custom providers
- Document error handling patterns
- Include performance considerations

Co-authored-by: Jane Developer <jane@example.com>
```

### 8.4 Release Process

#### Version Numbering
Follow Semantic Versioning (SemVer):
- `MAJOR.MINOR.PATCH`
- `MAJOR`: Breaking changes
- `MINOR`: New features (backward compatible)
- `PATCH`: Bug fixes (backward compatible)

#### Release Checklist
1. **Pre-Release**:
   - [ ] All tests pass
   - [ ] Documentation updated
   - [ ] Version numbers updated
   - [ ] Changelog updated
   - [ ] Performance benchmarks run

2. **Release Branch**:
   ```bash
   git checkout develop
   git checkout -b release/v2.1.0
   # Update version numbers
   # Final testing
   git checkout main
   git merge release/v2.1.0
   git tag v2.1.0
   ```

3. **Post-Release**:
   - [ ] Merge back to develop
   - [ ] Create GitHub release
   - [ ] Update documentation site
   - [ ] Notify stakeholders

#### Hotfix Process
For critical production issues:
```bash
git checkout main
git checkout -b hotfix/critical-audio-bug
# Fix the issue
# Test thoroughly
git checkout main
git merge hotfix/critical-audio-bug
git tag v2.0.1
git checkout develop
git merge hotfix/critical-audio-bug
```

---

## Document Information

**Document Version:** 2.0.0  
**Last Updated:** July 2025  
**Document Owner:** Development Team  
**Review Cycle:** Monthly  

### Additional Resources

- **API Documentation**: [API_Documentation.md](API_Documentation.md)
- **Testing Guide**: [Testing_Documentation.md](Testing_Documentation.md)
- **Architecture Overview**: [Detailed_Design_Document.md](Detailed_Design_Document.md)
- **Coding Standards**: Internal wiki
- **Issue Tracker**: GitHub Issues
- **CI/CD Pipeline**: GitHub Actions

### Contact Information

- **Lead Developer**: lead.dev@teamstranscription.com
- **Architecture Team**: architecture@teamstranscription.com
- **Developer Support**: dev.support@teamstranscription.com

---

*© 2025 Teams Transcription Application. All rights reserved. This documentation is provided for authorized developers only.*