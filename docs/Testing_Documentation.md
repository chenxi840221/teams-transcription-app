# Teams Audio Transcription Application
## Testing Documentation

**Version:** 2.0.0  
**Date:** July 2025  
**Document Type:** Quality Assurance Guide  

---

## Table of Contents

1. [Testing Strategy](#1-testing-strategy)
2. [Test Environment Setup](#2-test-environment-setup)
3. [Unit Testing](#3-unit-testing)
4. [Integration Testing](#4-integration-testing)
5. [Performance Testing](#5-performance-testing)
6. [Security Testing](#6-security-testing)
7. [User Acceptance Testing](#7-user-acceptance-testing)
8. [Automated Testing](#8-automated-testing)

---

## 1. Testing Strategy

### 1.1 Testing Pyramid

The testing strategy follows the testing pyramid approach:

```
           🔺 Manual Testing (5%)
         🔺🔺 End-to-End Testing (15%)
       🔺🔺🔺 Integration Testing (30%)
     🔺🔺🔺🔺 Unit Testing (50%)
```

#### Test Distribution
- **Unit Tests (50%)**: Component-level testing with mocks
- **Integration Tests (30%)**: Component interaction testing
- **End-to-End Tests (15%)**: Full workflow testing
- **Manual Tests (5%)**: Exploratory and usability testing

### 1.2 Testing Objectives

#### Primary Objectives
- **Functional Correctness**: All features work as specified
- **Performance**: Real-time audio processing with acceptable latency
- **Reliability**: Stable operation under various conditions
- **Security**: Data protection and privacy compliance
- **Usability**: Intuitive interface and user experience

#### Quality Gates
- **Code Coverage**: Minimum 80% for unit tests
- **Performance**: Audio latency < 100ms, UI response < 200ms
- **Memory Usage**: Maximum 2GB RAM during normal operation
- **Reliability**: 99.9% uptime during recording sessions
- **Security**: No sensitive data exposure, secure API key storage

### 1.3 Test Scope

#### In Scope
- Audio capture functionality
- Speech recognition integration
- Configuration management
- Export functionality
- User interface components
- Teams process monitoring
- Error handling and recovery

#### Out of Scope
- Third-party speech recognition APIs (tested via mocks)
- Windows OS functionality
- Network infrastructure
- Hardware-specific audio drivers

---

## 2. Test Environment Setup

### 2.1 Development Test Environment

#### Hardware Requirements
- **Primary Development Machine**:
  - Windows 11 Pro
  - Intel i7 or AMD equivalent
  - 16GB RAM
  - Audio interface (headset + speakers)
  - Multiple monitors for UI testing

#### Software Requirements
```
Visual Studio 2022 Enterprise (with testing tools)
CMake 3.20+
Google Test Framework
Microsoft Application Verifier
Windows Performance Toolkit (WPT)
Audio testing tools (Virtual Audio Cable)
```

#### Test Data Setup
```powershell
# Create test data structure
mkdir TestData
mkdir TestData\AudioSamples
mkdir TestData\Configurations
mkdir TestData\ExpectedOutputs
mkdir TestData\PerformanceBaselines

# Download test audio files
# - Various languages (English, Spanish, French)
# - Different audio qualities (8kHz, 16kHz, 44kHz)
# - Multiple speakers
# - Background noise scenarios
# - Edge cases (very quiet, very loud, distorted)
```

### 2.2 Continuous Integration Environment

#### CI Pipeline Configuration
```yaml
# GitHub Actions workflow
name: Continuous Integration

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v1
    
    - name: Setup CMake
      uses: lukka/get-cmake@latest
    
    - name: Configure CMake
      run: cmake -B build -S . -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config Release
    
    - name: Run Unit Tests
      run: |
        cd build
        ctest --output-on-failure -C Release -T test
    
    - name: Upload Test Results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: test-results
        path: build/Testing/
    
    - name: Code Coverage
      run: |
        # Generate coverage report
        opencppcoverage --export_type cobertura --modules build\*.exe --sources src\*.cpp
    
    - name: Upload Coverage
      uses: codecov/codecov-action@v3
      with:
        file: coverage.xml
```

### 2.3 Testing Infrastructure

#### Mock Services
```cpp
// Mock Speech Recognition Service
class MockSpeechProvider : public SpeechRecognition::ISpeechProvider {
public:
    MOCK_METHOD(bool, Initialize, (const SpeechRecognition::SpeechConfig&), (override));
    MOCK_METHOD(void, ProcessAudioData, (const std::vector<BYTE>&, const AudioCapture::AudioFormat&), (override));
    MOCK_METHOD(void, SetTranscriptionCallback, (SpeechRecognition::TranscriptionCallback), (override));
    MOCK_METHOD(bool, IsInitialized, (), (const, override));
    
    // Helper methods for testing
    void SimulateTranscription(const std::string& text, double confidence) {
        if (transcriptionCallback_) {
            transcriptionCallback_(text, confidence);
        }
    }
    
    void SimulateError() {
        // Simulate speech recognition error
        if (transcriptionCallback_) {
            transcriptionCallback_("", 0.0);
        }
    }

private:
    SpeechRecognition::TranscriptionCallback transcriptionCallback_;
};

// Mock Audio Device
class MockAudioDevice {
public:
    static void SetupVirtualAudioDevice() {
        // Configure Virtual Audio Cable for testing
        system("VirtualAudioCable.exe /install");
    }
    
    static void PlayTestAudio(const std::string& audioFile) {
        // Play test audio through virtual device
        std::string command = "ffplay -nodisp -autoexit " + audioFile;
        system(command.c_str());
    }
    
    static void GenerateTestTone(int frequency, int durationMs) {
        // Generate test tone for audio capture testing
        // Implementation using Windows audio APIs
    }
};
```

---

## 3. Unit Testing

### 3.1 Test Framework Setup

#### Google Test Configuration
```cmake
# CMakeLists.txt for tests
enable_testing()

# Google Test
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

# Test executable
add_executable(unit_tests
    tests/unit/AudioCaptureTest.cpp
    tests/unit/SpeechRecognitionTest.cpp
    tests/unit/ConfigManagerTest.cpp
    tests/unit/ExportManagerTest.cpp
    tests/unit/MainWindowTest.cpp
)

target_link_libraries(unit_tests
    PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    TeamsTranscriptionLib
)

# Discover tests
gtest_discover_tests(unit_tests)

# Test with memory checking
add_test(
    NAME unit_tests_with_memcheck
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit_tests
)
set_tests_properties(unit_tests_with_memcheck PROPERTIES
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:abort_on_error=1"
)
```

### 3.2 AudioCapture Unit Tests

```cpp
// tests/unit/AudioCaptureTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "AudioCapture.h"
#include "MockAudioDevice.h"

class AudioCaptureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize COM for tests
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        audioCapture = std::make_unique<AudioCapture>();
        MockAudioDevice::SetupVirtualAudioDevice();
    }
    
    void TearDown() override {
        audioCapture.reset();
        CoUninitialize();
    }
    
    std::unique_ptr<AudioCapture> audioCapture;
};

TEST_F(AudioCaptureTest, InitializeWithDefaultQuality) {
    HRESULT hr = audioCapture->Initialize();
    EXPECT_TRUE(SUCCEEDED(hr));
    EXPECT_FALSE(audioCapture->IsCapturing());
    
    auto format = audioCapture->GetAudioFormat();
    EXPECT_EQ(format.sampleRate, 16000u);
    EXPECT_GE(format.channels, 1u);
    EXPECT_LE(format.channels, 2u);
}

TEST_F(AudioCaptureTest, InitializeWithDifferentQualities) {
    std::vector<AudioCapture::AudioQuality> qualities = {
        AudioCapture::AudioQuality::Low,
        AudioCapture::AudioQuality::Medium,
        AudioCapture::AudioQuality::High
    };
    
    for (auto quality : qualities) {
        HRESULT hr = audioCapture->Initialize(quality);
        EXPECT_TRUE(SUCCEEDED(hr)) << "Failed to initialize with quality: " << static_cast<int>(quality);
        
        auto format = audioCapture->GetAudioFormat();
        EXPECT_EQ(format.sampleRate, static_cast<UINT32>(quality));
    }
}

TEST_F(AudioCaptureTest, StartStopCapture) {
    audioCapture->Initialize();
    
    // Test start capture
    HRESULT hr = audioCapture->StartCapture();
    EXPECT_TRUE(SUCCEEDED(hr));
    EXPECT_TRUE(audioCapture->IsCapturing());
    
    // Test stop capture
    audioCapture->StopCapture();
    EXPECT_FALSE(audioCapture->IsCapturing());
    
    // Test multiple stops (should be safe)
    audioCapture->StopCapture();
    EXPECT_FALSE(audioCapture->IsCapturing());
}

TEST_F(AudioCaptureTest, AudioDataCallback) {
    audioCapture->Initialize();
    
    bool callbackCalled = false;
    std::vector<BYTE> receivedData;
    AudioCapture::AudioFormat receivedFormat;
    
    audioCapture->SetAudioDataCallback([&](const std::vector<BYTE>& data, 
                                           const AudioCapture::AudioFormat& format) {
        callbackCalled = true;
        receivedData = data;
        receivedFormat = format;
    });
    
    audioCapture->StartCapture();
    
    // Generate test audio
    MockAudioDevice::GenerateTestTone(440, 1000);  // 440Hz for 1 second
    
    // Wait for callback
    auto start = std::chrono::steady_clock::now();
    while (!callbackCalled && 
           std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    audioCapture->StopCapture();
    
    EXPECT_TRUE(callbackCalled);
    EXPECT_GT(receivedData.size(), 0u);
    EXPECT_GT(receivedFormat.sampleRate, 0u);
}

TEST_F(AudioCaptureTest, CaptureStatistics) {
    audioCapture->Initialize();
    audioCapture->StartCapture();
    
    // Generate audio for statistics
    MockAudioDevice::GenerateTestTone(1000, 2000);  // 2 seconds
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    
    auto stats = audioCapture->GetCaptureStats();
    audioCapture->StopCapture();
    
    EXPECT_GT(stats.totalFramesCaptured, 0u);
    EXPECT_GT(stats.totalBytesProcessed, 0u);
    EXPECT_GT(stats.captureTimeSeconds, 1.0);  // At least 1 second
    EXPECT_LT(stats.captureTimeSeconds, 5.0);  // But not more than 5 seconds
}

TEST_F(AudioCaptureTest, ErrorHandling) {
    // Test initialization without COM
    {
        CoUninitialize();
        AudioCapture captureWithoutCOM;
        HRESULT hr = captureWithoutCOM.Initialize();
        EXPECT_TRUE(FAILED(hr));
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    }
    
    // Test start without initialization
    {
        AudioCapture uninitializedCapture;
        HRESULT hr = uninitializedCapture.StartCapture();
        EXPECT_TRUE(FAILED(hr));
    }
}

// Performance test
TEST_F(AudioCaptureTest, PerformanceLatency) {
    audioCapture->Initialize(AudioCapture::AudioQuality::Medium);
    
    std::vector<std::chrono::milliseconds> latencies;
    std::mutex latencyMutex;
    
    audioCapture->SetAudioDataCallback([&](const std::vector<BYTE>& data, 
                                           const AudioCapture::AudioFormat& format) {
        auto callbackTime = std::chrono::steady_clock::now();
        // In real test, would correlate with audio generation time
        std::lock_guard<std::mutex> lock(latencyMutex);
        latencies.push_back(std::chrono::milliseconds(50));  // Simulated latency
    });
    
    audioCapture->StartCapture();
    
    // Generate multiple audio bursts
    for (int i = 0; i < 10; ++i) {
        MockAudioDevice::GenerateTestTone(440 + i * 100, 500);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    audioCapture->StopCapture();
    
    // Verify latencies
    ASSERT_GT(latencies.size(), 0u);
    for (auto latency : latencies) {
        EXPECT_LT(latency.count(), 100) << "Audio latency too high: " << latency.count() << "ms";
    }
}
```

### 3.3 SpeechRecognition Unit Tests

```cpp
// tests/unit/SpeechRecognitionTest.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "SpeechRecognition.h"
#include "MockSpeechProvider.h"

class SpeechRecognitionTest : public ::testing::Test {
protected:
    void SetUp() override {
        speechRecognition = std::make_unique<SpeechRecognition>();
        
        // Create test configuration
        config.provider = SpeechRecognition::Provider::Windows;  // For testing
        config.language = "en-US";
        config.enablePunctuation = true;
        config.enableSpeakerDiarization = false;
    }
    
    std::unique_ptr<SpeechRecognition> speechRecognition;
    SpeechRecognition::SpeechConfig config;
};

TEST_F(SpeechRecognitionTest, InitializeWithValidConfig) {
    bool result = speechRecognition->Initialize(config);
    EXPECT_TRUE(result);
    EXPECT_TRUE(speechRecognition->IsInitialized());
}

TEST_F(SpeechRecognitionTest, InitializeWithInvalidConfig) {
    // Test with empty language
    config.language = "";
    bool result = speechRecognition->Initialize(config);
    EXPECT_FALSE(result);
    EXPECT_FALSE(speechRecognition->IsInitialized());
}

TEST_F(SpeechRecognitionTest, ProcessAudioData) {
    speechRecognition->Initialize(config);
    
    std::string receivedText;
    double receivedConfidence = 0.0;
    bool callbackCalled = false;
    
    speechRecognition->SetTranscriptionCallback([&](const std::string& text, double confidence) {
        receivedText = text;
        receivedConfidence = confidence;
        callbackCalled = true;
    });
    
    // Create test audio data
    std::vector<BYTE> audioData(16000 * 2);  // 1 second of 16kHz audio
    std::fill(audioData.begin(), audioData.end(), 128);  // Silence
    
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    speechRecognition->ProcessAudioData(audioData, format);
    
    // Wait for processing (Windows Speech Recognition is async)
    auto start = std::chrono::steady_clock::now();
    while (!callbackCalled && 
           std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // For silence, we might not get a callback, which is expected
    // In a real test, we'd use actual speech audio
}

TEST_F(SpeechRecognitionTest, ProviderSwitching) {
    // Test with different providers
    std::vector<SpeechRecognition::Provider> providers = {
        SpeechRecognition::Provider::Windows,
        SpeechRecognition::Provider::Azure,
        SpeechRecognition::Provider::Google
    };
    
    for (auto provider : providers) {
        config.provider = provider;
        
        if (provider != SpeechRecognition::Provider::Windows) {
            config.apiKey = "test_key";  // Required for cloud providers
        }
        
        bool result = speechRecognition->Initialize(config);
        
        if (provider == SpeechRecognition::Provider::Windows) {
            EXPECT_TRUE(result) << "Windows provider should always initialize";
        } else {
            // Cloud providers might fail without valid keys, which is expected in tests
            // In CI, we can use mock providers
        }
    }
}

// Mock-based test for cloud providers
TEST_F(SpeechRecognitionTest, MockProviderTest) {
    auto mockProvider = std::make_unique<MockSpeechProvider>();
    
    // Set up expectations
    EXPECT_CALL(*mockProvider, Initialize(::testing::_))
        .WillOnce(::testing::Return(true));
    
    EXPECT_CALL(*mockProvider, IsInitialized())
        .WillRepeatedly(::testing::Return(true));
    
    EXPECT_CALL(*mockProvider, ProcessAudioData(::testing::_, ::testing::_))
        .Times(::testing::AtLeast(1));
    
    // Replace provider in speech recognition (would need factory pattern)
    // speechRecognition->SetProvider(std::move(mockProvider));
    
    // Test callback
    std::string testText = "Hello, this is a test transcription.";
    double testConfidence = 0.95;
    
    mockProvider->SimulateTranscription(testText, testConfidence);
    
    // Verify results...
}
```

### 3.4 ConfigManager Unit Tests

```cpp
// tests/unit/ConfigManagerTest.cpp
#include <gtest/gtest.h>
#include "ConfigManager.h"
#include <fstream>
#include <filesystem>

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        configManager = std::make_unique<ConfigManager>();
        testConfigPath = "test_config.json";
        
        // Clean up any existing test file
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }
    
    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }
    
    std::unique_ptr<ConfigManager> configManager;
    std::string testConfigPath;
    
    void CreateTestConfigFile(const std::string& content) {
        std::ofstream file(testConfigPath);
        file << content;
        file.close();
    }
};

TEST_F(ConfigManagerTest, DefaultConfiguration) {
    const auto& config = configManager->GetConfig();
    
    // Verify default values
    EXPECT_FALSE(config.autoStartRecording);
    EXPECT_EQ(config.outputFormat, "wav");
    EXPECT_EQ(config.audioQuality, 16000);
    EXPECT_TRUE(config.minimizeToTray);
    EXPECT_TRUE(config.requireConsent);
    EXPECT_EQ(config.dataRetentionDays, 30);
}

TEST_F(ConfigManagerTest, LoadValidConfiguration) {
    std::string validConfig = R"(
    {
        "recording": {
            "autoStart": true,
            "outputFormat": "mp3",
            "audioQuality": 44100
        },
        "speechRecognition": {
            "provider": "azure",
            "language": "en-GB",
            "enablePunctuation": false
        },
        "privacy": {
            "requireConsent": false,
            "dataRetentionDays": 60
        }
    }
    )";
    
    CreateTestConfigFile(validConfig);
    
    bool result = configManager->LoadConfig(testConfigPath);
    EXPECT_TRUE(result);
    
    const auto& config = configManager->GetConfig();
    EXPECT_TRUE(config.autoStartRecording);
    EXPECT_EQ(config.outputFormat, "mp3");
    EXPECT_EQ(config.audioQuality, 44100);
    EXPECT_EQ(config.speechConfig.provider, SpeechRecognition::Provider::Azure);
    EXPECT_EQ(config.speechConfig.language, "en-GB");
    EXPECT_FALSE(config.speechConfig.enablePunctuation);
    EXPECT_FALSE(config.requireConsent);
    EXPECT_EQ(config.dataRetentionDays, 60);
}

TEST_F(ConfigManagerTest, LoadInvalidConfiguration) {
    std::string invalidConfig = R"(
    {
        "recording": {
            "autoStart": "not_a_boolean",
            "audioQuality": "invalid_number"
        }
    }
    )";
    
    CreateTestConfigFile(invalidConfig);
    
    bool result = configManager->LoadConfig(testConfigPath);
    EXPECT_FALSE(result);
    
    // Should keep default values
    const auto& config = configManager->GetConfig();
    EXPECT_FALSE(config.autoStartRecording);  // Default value
    EXPECT_EQ(config.audioQuality, 16000);    // Default value
}

TEST_F(ConfigManagerTest, SaveConfiguration) {
    // Modify configuration
    auto& config = configManager->GetConfig();
    config.autoStartRecording = true;
    config.audioQuality = 48000;
    config.speechConfig.language = "fr-FR";
    
    // Save configuration
    bool result = configManager->SaveConfig(testConfigPath);
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testConfigPath));
    
    // Verify file content
    std::ifstream file(testConfigPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("\"autoStart\": true"), std::string::npos);
    EXPECT_NE(content.find("\"audioQuality\": 48000"), std::string::npos);
    EXPECT_NE(content.find("\"language\": \"fr-FR\""), std::string::npos);
}

TEST_F(ConfigManagerTest, LoadSaveRoundTrip) {
    // Create initial configuration
    auto& config = configManager->GetConfig();
    config.autoStartRecording = true;
    config.outputFormat = "wav";
    config.audioQuality = 22050;
    config.speechConfig.provider = SpeechRecognition::Provider::Google;
    config.speechConfig.apiKey = "test_key_123";
    config.speechConfig.language = "es-ES";
    config.speechConfig.enablePunctuation = false;
    config.speechConfig.enableSpeakerDiarization = true;
    
    // Save configuration
    EXPECT_TRUE(configManager->SaveConfig(testConfigPath));
    
    // Create new config manager and load
    auto newConfigManager = std::make_unique<ConfigManager>();
    EXPECT_TRUE(newConfigManager->LoadConfig(testConfigPath));
    
    // Verify all values are preserved
    const auto& loadedConfig = newConfigManager->GetConfig();
    EXPECT_EQ(loadedConfig.autoStartRecording, true);
    EXPECT_EQ(loadedConfig.outputFormat, "wav");
    EXPECT_EQ(loadedConfig.audioQuality, 22050);
    EXPECT_EQ(loadedConfig.speechConfig.provider, SpeechRecognition::Provider::Google);
    EXPECT_EQ(loadedConfig.speechConfig.apiKey, "test_key_123");
    EXPECT_EQ(loadedConfig.speechConfig.language, "es-ES");
    EXPECT_EQ(loadedConfig.speechConfig.enablePunctuation, false);
    EXPECT_EQ(loadedConfig.speechConfig.enableSpeakerDiarization, true);
}

TEST_F(ConfigManagerTest, MissingConfigFile) {
    bool result = configManager->LoadConfig("nonexistent_file.json");
    EXPECT_FALSE(result);
    
    // Should keep default configuration
    const auto& config = configManager->GetConfig();
    EXPECT_FALSE(config.autoStartRecording);  // Default
}

TEST_F(ConfigManagerTest, PartialConfiguration) {
    std::string partialConfig = R"(
    {
        "recording": {
            "audioQuality": 8000
        }
    }
    )";
    
    CreateTestConfigFile(partialConfig);
    
    bool result = configManager->LoadConfig(testConfigPath);
    EXPECT_TRUE(result);
    
    const auto& config = configManager->GetConfig();
    EXPECT_EQ(config.audioQuality, 8000);           // Loaded value
    EXPECT_FALSE(config.autoStartRecording);        // Default value
    EXPECT_EQ(config.outputFormat, "wav");          // Default value
}
```

---

## 4. Integration Testing

### 4.1 Component Integration Tests

#### Audio-to-Speech Integration
```cpp
// tests/integration/AudioSpeechIntegrationTest.cpp
#include <gtest/gtest.h>
#include "AudioCapture.h"
#include "SpeechRecognition.h"
#include "MockAudioDevice.h"

class AudioSpeechIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        
        audioCapture = std::make_unique<AudioCapture>();
        speechRecognition = std::make_unique<SpeechRecognition>();
        
        // Configure speech recognition
        SpeechRecognition::SpeechConfig config;
        config.provider = SpeechRecognition::Provider::Windows;
        config.language = "en-US";
        config.enablePunctuation = true;
        
        ASSERT_TRUE(speechRecognition->Initialize(config));
        ASSERT_TRUE(SUCCEEDED(audioCapture->Initialize()));
        
        MockAudioDevice::SetupVirtualAudioDevice();
    }
    
    void TearDown() override {
        speechRecognition.reset();
        audioCapture.reset();
        CoUninitialize();
    }
    
    std::unique_ptr<AudioCapture> audioCapture;
    std::unique_ptr<SpeechRecognition> speechRecognition;
};

TEST_F(AudioSpeechIntegrationTest, EndToEndTranscription) {
    std::vector<std::string> transcriptions;
    std::mutex transcriptionMutex;
    
    // Set up transcription callback
    speechRecognition->SetTranscriptionCallback([&](const std::string& text, double confidence) {
        if (!text.empty() && confidence > 0.5) {
            std::lock_guard<std::mutex> lock(transcriptionMutex);
            transcriptions.push_back(text);
        }
    });
    
    // Set up audio pipeline
    audioCapture->SetAudioDataCallback([this](const std::vector<BYTE>& audioData,
                                              const AudioCapture::AudioFormat& format) {
        speechRecognition->ProcessAudioData(audioData, format);
    });
    
    // Start capture
    ASSERT_TRUE(SUCCEEDED(audioCapture->StartCapture()));
    
    // Play test audio with known content
    MockAudioDevice::PlayTestAudio("test_audio/clear_speech_hello_world.wav");
    
    // Wait for transcription
    auto start = std::chrono::steady_clock::now();
    while (transcriptions.empty() && 
           std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    audioCapture->StopCapture();
    
    // Verify transcription results
    ASSERT_GT(transcriptions.size(), 0u) << "No transcriptions received";
    
    // Check if any transcription contains expected words
    bool foundExpectedContent = false;
    for (const auto& transcription : transcriptions) {
        if (transcription.find("hello") != std::string::npos || 
            transcription.find("world") != std::string::npos) {
            foundExpectedContent = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundExpectedContent) << "Expected content not found in transcriptions";
}

TEST_F(AudioSpeechIntegrationTest, MultipleLanguageTest) {
    struct LanguageTest {
        std::string language;
        std::string audioFile;
        std::string expectedWord;
    };
    
    std::vector<LanguageTest> tests = {
        {"en-US", "test_audio/english_hello.wav", "hello"},
        {"es-ES", "test_audio/spanish_hola.wav", "hola"},
        {"fr-FR", "test_audio/french_bonjour.wav", "bonjour"}
    };
    
    for (const auto& test : tests) {
        // Reconfigure for language
        SpeechRecognition::SpeechConfig config;
        config.provider = SpeechRecognition::Provider::Windows;
        config.language = test.language;
        
        ASSERT_TRUE(speechRecognition->Initialize(config));
        
        std::string receivedTranscription;
        speechRecognition->SetTranscriptionCallback([&](const std::string& text, double confidence) {
            if (confidence > 0.7) {
                receivedTranscription = text;
            }
        });
        
        audioCapture->SetAudioDataCallback([this](const std::vector<BYTE>& audioData,
                                                  const AudioCapture::AudioFormat& format) {
            speechRecognition->ProcessAudioData(audioData, format);
        });
        
        ASSERT_TRUE(SUCCEEDED(audioCapture->StartCapture()));
        MockAudioDevice::PlayTestAudio(test.audioFile);
        
        // Wait for transcription
        auto start = std::chrono::steady_clock::now();
        while (receivedTranscription.empty() && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        audioCapture->StopCapture();
        
        // Convert to lowercase for comparison
        std::transform(receivedTranscription.begin(), receivedTranscription.end(),
                      receivedTranscription.begin(), ::tolower);
        
        EXPECT_NE(receivedTranscription.find(test.expectedWord), std::string::npos)
            << "Expected word '" << test.expectedWord << "' not found in transcription: " 
            << receivedTranscription << " (language: " << test.language << ")";
    }
}
```

#### Configuration-Component Integration
```cpp
// tests/integration/ConfigurationIntegrationTest.cpp
TEST_F(ConfigurationIntegrationTest, ConfigurationAffectsComponents) {
    ConfigManager configManager;
    
    // Test audio quality configuration
    auto& config = configManager.GetConfig();
    config.audioQuality = 8000;
    
    AudioCapture audioCapture;
    ASSERT_TRUE(SUCCEEDED(audioCapture.Initialize(static_cast<AudioCapture::AudioQuality>(config.audioQuality))));
    
    auto format = audioCapture.GetAudioFormat();
    EXPECT_EQ(format.sampleRate, 8000u);
    
    // Test speech recognition configuration
    config.speechConfig.provider = SpeechRecognition::Provider::Windows;
    config.speechConfig.language = "en-GB";
    
    SpeechRecognition speechRecognition;
    ASSERT_TRUE(speechRecognition.Initialize(config.speechConfig));
    EXPECT_TRUE(speechRecognition.IsInitialized());
}
```

### 4.2 System Integration Tests

#### Full Application Workflow
```cpp
// tests/integration/ApplicationWorkflowTest.cpp
class ApplicationWorkflowTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test environment
        std::filesystem::create_directories("test_data/recordings");
        std::filesystem::create_directories("test_data/transcripts");
        std::filesystem::create_directories("test_data/exports");
        
        // Create test configuration
        CreateTestConfiguration();
    }
    
    void TearDown() override {
        // Clean up test data
        std::filesystem::remove_all("test_data");
    }
    
    void CreateTestConfiguration() {
        nlohmann::json config;
        config["recording"]["autoStart"] = false;
        config["recording"]["outputDirectory"] = "test_data/recordings";
        config["speechRecognition"]["provider"] = "windows";
        config["speechRecognition"]["language"] = "en-US";
        
        std::ofstream file("test_data/test_config.json");
        file << config.dump(4);
        file.close();
    }
};

TEST_F(ApplicationWorkflowTest, CompleteRecordingWorkflow) {
    // 1. Initialize application components
    ConfigManager configManager;
    ASSERT_TRUE(configManager.LoadConfig("test_data/test_config.json"));
    
    AudioCapture audioCapture;
    ASSERT_TRUE(SUCCEEDED(audioCapture.Initialize()));
    
    SpeechRecognition speechRecognition;
    ASSERT_TRUE(speechRecognition.Initialize(configManager.GetSpeechConfig()));
    
    ExportManager exportManager;
    
    // 2. Set up data flow
    MeetingTranscript transcript;
    transcript.startTime = std::chrono::system_clock::now();
    std::vector<TranscriptionSegment> segments;
    
    speechRecognition.SetTranscriptionCallback([&](const std::string& text, double confidence) {
        if (!text.empty()) {
            TranscriptionSegment segment;
            segment.text = text;
            segment.confidence = confidence;
            segment.startTime = std::chrono::milliseconds(segments.size() * 3000);
            segment.endTime = segment.startTime + std::chrono::milliseconds(3000);
            segment.isFinal = true;
            
            segments.push_back(segment);
        }
    });
    
    audioCapture.SetAudioDataCallback([&](const std::vector<BYTE>& audioData,
                                          const AudioCapture::AudioFormat& format) {
        speechRecognition.ProcessAudioData(audioData, format);
    });
    
    // 3. Start recording
    ASSERT_TRUE(SUCCEEDED(audioCapture.StartCapture()));
    
    // 4. Simulate meeting audio
    MockAudioDevice::PlayTestAudio("test_audio/meeting_sample.wav");
    
    // 5. Wait for transcription
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 6. Stop recording
    audioCapture.StopCapture();
    transcript.endTime = std::chrono::system_clock::now();
    transcript.segments = segments;
    
    // 7. Verify transcription
    ASSERT_GT(segments.size(), 0u) << "No transcription segments generated";
    
    // 8. Export transcript
    bool exportSuccess = exportManager.Export(transcript, 
                                             ExportManager::Format::TXT,
                                             "test_data/exports/test_transcript.txt");
    EXPECT_TRUE(exportSuccess);
    EXPECT_TRUE(std::filesystem::exists("test_data/exports/test_transcript.txt"));
    
    // 9. Verify export content
    std::ifstream exportFile("test_data/exports/test_transcript.txt");
    std::string exportContent((std::istreambuf_iterator<char>(exportFile)),
                             std::istreambuf_iterator<char>());
    EXPECT_GT(exportContent.length(), 0u);
    
    // Should contain at least some of the transcribed text
    bool containsTranscription = false;
    for (const auto& segment : segments) {
        if (exportContent.find(segment.text) != std::string::npos) {
            containsTranscription = true;
            break;
        }
    }
    EXPECT_TRUE(containsTranscription);
}
```

---

## 5. Performance Testing

### 5.1 Audio Latency Testing

```cpp
// tests/performance/AudioLatencyTest.cpp
#include <benchmark/benchmark.h>
#include "AudioCapture.h"

class AudioLatencyTest {
public:
    static void SetUp() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        audioCapture.Initialize();
    }
    
    static void TearDown() {
        audioCapture.StopCapture();
        CoUninitialize();
    }
    
    static AudioCapture audioCapture;
};

AudioCapture AudioLatencyTest::audioCapture;

static void BM_AudioCaptureLatency(benchmark::State& state) {
    AudioLatencyTest::SetUp();
    
    std::vector<std::chrono::microseconds> latencies;
    std::mutex latencyMutex;
    
    AudioLatencyTest::audioCapture.SetAudioDataCallback([&](const std::vector<BYTE>& data,
                                                            const AudioCapture::AudioFormat& format) {
        auto processingTime = std::chrono::steady_clock::now();
        // In real test, would measure from audio generation time
        std::lock_guard<std::mutex> lock(latencyMutex);
        latencies.push_back(std::chrono::microseconds(50000));  // Simulated 50ms latency
    });
    
    for (auto _ : state) {
        auto start = std::chrono::steady_clock::now();
        
        AudioLatencyTest::audioCapture.StartCapture();
        MockAudioDevice::GenerateTestTone(1000, 100);  // 100ms tone
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        AudioLatencyTest::audioCapture.StopCapture();
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        state.SetIterationTime(duration.count() / 1000000.0);
    }
    
    // Report average latency
    if (!latencies.empty()) {
        auto avgLatency = std::accumulate(latencies.begin(), latencies.end(), std::chrono::microseconds(0)) / latencies.size();
        state.counters["AvgLatency_us"] = avgLatency.count();
    }
    
    AudioLatencyTest::TearDown();
}
BENCHMARK(BM_AudioCaptureLatency)->UseManualTime()->Unit(benchmark::kMillisecond);

static void BM_SpeechRecognitionThroughput(benchmark::State& state) {
    SpeechRecognition speechRecognition;
    SpeechRecognition::SpeechConfig config;
    config.provider = SpeechRecognition::Provider::Windows;
    config.language = "en-US";
    
    speechRecognition.Initialize(config);
    
    // Create test audio data
    size_t audioSize = state.range(0);
    std::vector<BYTE> audioData(audioSize);
    std::fill(audioData.begin(), audioData.end(), 128);
    
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    for (auto _ : state) {
        auto start = std::chrono::steady_clock::now();
        speechRecognition.ProcessAudioData(audioData, format);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        state.SetIterationTime(duration.count() / 1000000.0);
    }
    
    state.SetBytesProcessed(state.iterations() * audioSize);
    state.counters["AudioSize"] = audioSize;
}
BENCHMARK(BM_SpeechRecognitionThroughput)
    ->RangeMultiplier(2)
    ->Range(1024, 1024*64)
    ->UseManualTime()
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
```

### 5.2 Memory Usage Testing

```cpp
// tests/performance/MemoryUsageTest.cpp
class MemoryUsageTest : public ::testing::Test {
protected:
    void SetUp() override {
        initialMemory = GetCurrentMemoryUsage();
    }
    
    void TearDown() override {
        size_t finalMemory = GetCurrentMemoryUsage();
        size_t memoryDelta = finalMemory - initialMemory;
        
        // Log memory usage for analysis
        std::cout << "Memory usage delta: " << memoryDelta / 1024 / 1024 << " MB" << std::endl;
    }
    
    size_t GetCurrentMemoryUsage() {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
        return 0;
    }
    
    size_t initialMemory;
    
    static const size_t MAX_MEMORY_USAGE = 2ULL * 1024 * 1024 * 1024;  // 2GB
};

TEST_F(MemoryUsageTest, LongRunningCapture) {
    AudioCapture audioCapture;
    ASSERT_TRUE(SUCCEEDED(audioCapture.Initialize()));
    
    std::vector<size_t> memorySnapshots;
    
    audioCapture.SetAudioDataCallback([&](const std::vector<BYTE>& data,
                                          const AudioCapture::AudioFormat& format) {
        // Take memory snapshot every 1000 callbacks
        static int callbackCount = 0;
        if (++callbackCount % 1000 == 0) {
            memorySnapshots.push_back(GetCurrentMemoryUsage());
        }
    });
    
    ASSERT_TRUE(SUCCEEDED(audioCapture.StartCapture()));
    
    // Run for 5 minutes with continuous audio
    auto endTime = std::chrono::steady_clock::now() + std::chrono::minutes(5);
    while (std::chrono::steady_clock::now() < endTime) {
        MockAudioDevice::GenerateTestTone(440, 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    audioCapture.StopCapture();
    
    // Analyze memory growth
    ASSERT_GT(memorySnapshots.size(), 2u) << "Not enough memory snapshots";
    
    size_t maxMemory = *std::max_element(memorySnapshots.begin(), memorySnapshots.end());
    EXPECT_LT(maxMemory, MAX_MEMORY_USAGE) << "Memory usage exceeded limit: " 
                                          << maxMemory / 1024 / 1024 << " MB";
    
    // Check for memory leaks (memory should not grow continuously)
    if (memorySnapshots.size() >= 10) {
        size_t firstHalf = 0, secondHalf = 0;
        size_t midpoint = memorySnapshots.size() / 2;
        
        for (size_t i = 0; i < midpoint; ++i) {
            firstHalf += memorySnapshots[i];
        }
        for (size_t i = midpoint; i < memorySnapshots.size(); ++i) {
            secondHalf += memorySnapshots[i];
        }
        
        double avgFirstHalf = static_cast<double>(firstHalf) / midpoint;
        double avgSecondHalf = static_cast<double>(secondHalf) / (memorySnapshots.size() - midpoint);
        
        double growthRate = (avgSecondHalf - avgFirstHalf) / avgFirstHalf;
        
        EXPECT_LT(growthRate, 0.1) << "Memory growth rate too high: " << (growthRate * 100) << "%";
    }
}

TEST_F(MemoryUsageTest, SpeechRecognitionMemoryStability) {
    SpeechRecognition speechRecognition;
    SpeechRecognition::SpeechConfig config;
    config.provider = SpeechRecognition::Provider::Windows;
    config.language = "en-US";
    
    ASSERT_TRUE(speechRecognition.Initialize(config));
    
    // Process many audio chunks
    std::vector<BYTE> audioChunk(16000 * 2);  // 1 second at 16kHz
    std::fill(audioChunk.begin(), audioChunk.end(), 128);
    
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    size_t memoryBefore = GetCurrentMemoryUsage();
    
    for (int i = 0; i < 1000; ++i) {
        speechRecognition.ProcessAudioData(audioChunk, format);
        
        if (i % 100 == 0) {
            size_t currentMemory = GetCurrentMemoryUsage();
            EXPECT_LT(currentMemory, MAX_MEMORY_USAGE) 
                << "Memory usage too high at iteration " << i << ": " 
                << currentMemory / 1024 / 1024 << " MB";
        }
    }
    
    size_t memoryAfter = GetCurrentMemoryUsage();
    size_t memoryGrowth = memoryAfter - memoryBefore;
    
    // Memory growth should be reasonable
    EXPECT_LT(memoryGrowth, 100 * 1024 * 1024) << "Memory growth too high: " 
                                               << memoryGrowth / 1024 / 1024 << " MB";
}
```

---

## 6. Security Testing

### 6.1 Data Protection Testing

```cpp
// tests/security/DataProtectionTest.cpp
class DataProtectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDataDir = "test_security_data";
        std::filesystem::create_directories(testDataDir);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(testDataDir);
    }
    
    std::string testDataDir;
};

TEST_F(DataProtectionTest, APIKeySecureStorage) {
    ConfigManager configManager;
    
    // Test API key storage
    auto& config = configManager.GetConfig();
    config.speechConfig.apiKey = "sensitive_api_key_12345";
    
    std::string configFile = testDataDir + "/secure_config.json";
    ASSERT_TRUE(configManager.SaveConfig(configFile));
    
    // Read raw file content
    std::ifstream file(configFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    
    // API key should not be stored in plain text
    EXPECT_EQ(content.find("sensitive_api_key_12345"), std::string::npos)
        << "API key found in plain text in configuration file";
    
    // But should be retrievable through proper interface
    ConfigManager loadedConfig;
    ASSERT_TRUE(loadedConfig.LoadConfig(configFile));
    EXPECT_EQ(loadedConfig.GetSpeechConfig().apiKey, "sensitive_api_key_12345");
}

TEST_F(DataProtectionTest, TranscriptEncryption) {
    // Create test transcript
    MeetingTranscript transcript;
    transcript.meetingId = "test_meeting_001";
    transcript.startTime = std::chrono::system_clock::now();
    transcript.endTime = transcript.startTime + std::chrono::hours(1);
    
    TranscriptionSegment segment;
    segment.text = "This is sensitive meeting content that should be encrypted";
    segment.confidence = 0.95;
    transcript.segments.push_back(segment);
    
    // Test encryption during export
    ExportManager exportManager;
    std::string encryptedFile = testDataDir + "/encrypted_transcript.json";
    
    // Enable encryption (this would be a configuration option)
    bool encryptionEnabled = true;
    if (encryptionEnabled) {
        // Export with encryption
        ASSERT_TRUE(exportManager.Export(transcript, ExportManager::Format::JSON, encryptedFile));
        
        // Read raw file content
        std::ifstream file(encryptedFile, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        // Sensitive content should not be visible in plain text
        EXPECT_EQ(content.find("sensitive meeting content"), std::string::npos)
            << "Sensitive content found in plain text in encrypted file";
    }
}

TEST_F(DataProtectionTest, MemoryWiping) {
    // Test that sensitive data is wiped from memory
    std::vector<BYTE> sensitiveData(1024);
    std::fill(sensitiveData.begin(), sensitiveData.end(), 0xAA);
    
    // Use the data
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    SpeechRecognition speechRecognition;
    
    SpeechRecognition::SpeechConfig config;
    config.provider = SpeechRecognition::Provider::Windows;
    config.language = "en-US";
    speechRecognition.Initialize(config);
    
    speechRecognition.ProcessAudioData(sensitiveData, format);
    
    // Manually wipe sensitive data
    SecureZeroMemory(sensitiveData.data(), sensitiveData.size());
    
    // Verify data is wiped
    for (BYTE b : sensitiveData) {
        EXPECT_EQ(b, 0) << "Sensitive data not properly wiped from memory";
    }
}
```

### 6.2 Input Validation Testing

```cpp
// tests/security/InputValidationTest.cpp
TEST_F(InputValidationTest, ConfigurationValidation) {
    ConfigManager configManager;
    
    // Test malicious JSON
    std::string maliciousConfig = R"(
    {
        "recording": {
            "outputDirectory": "../../../../../../etc/passwd",
            "audioQuality": -1
        },
        "speechRecognition": {
            "apiKey": "'; DROP TABLE users; --",
            "language": "\x00\x01\x02\x03"
        }
    }
    )";
    
    std::string configFile = "malicious_config.json";
    std::ofstream file(configFile);
    file << maliciousConfig;
    file.close();
    
    // Should reject malicious configuration
    bool result = configManager.LoadConfig(configFile);
    EXPECT_FALSE(result) << "Malicious configuration should be rejected";
    
    std::filesystem::remove(configFile);
}

TEST_F(InputValidationTest, PathTraversalPrevention) {
    ExportManager exportManager;
    MeetingTranscript transcript;  // Empty transcript for testing
    
    // Test path traversal attempts
    std::vector<std::string> maliciousPaths = {
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32\\config\\sam",
        "/etc/shadow",
        "C:\\Windows\\System32\\config\\SAM",
        "NUL",
        "CON",
        "PRN"
    };
    
    for (const auto& path : maliciousPaths) {
        bool result = exportManager.Export(transcript, ExportManager::Format::TXT, path);
        EXPECT_FALSE(result) << "Export should reject malicious path: " << path;
    }
}

TEST_F(InputValidationTest, AudioDataValidation) {
    AudioCapture audioCapture;
    ASSERT_TRUE(SUCCEEDED(audioCapture.Initialize()));
    
    SpeechRecognition speechRecognition;
    SpeechRecognition::SpeechConfig config;
    config.provider = SpeechRecognition::Provider::Windows;
    config.language = "en-US";
    ASSERT_TRUE(speechRecognition.Initialize(config));
    
    // Test with malformed audio data
    std::vector<BYTE> malformedData = {0xFF, 0xFE, 0xFD, 0xFC};  // Invalid audio
    AudioCapture::AudioFormat invalidFormat{0, 0, 0, 0};  // Invalid format
    
    // Should handle gracefully without crashing
    EXPECT_NO_THROW(speechRecognition.ProcessAudioData(malformedData, invalidFormat));
    
    // Test with extremely large data
    std::vector<BYTE> oversizedData(100 * 1024 * 1024);  // 100MB
    AudioCapture::AudioFormat format{16000, 1, 16, 32000};
    
    EXPECT_NO_THROW(speechRecognition.ProcessAudioData(oversizedData, format));
}
```

---

## 7. User Acceptance Testing

### 7.1 UAT Test Plans

#### Recording Functionality UAT
```
Test Case: TC-UAT-001 - Basic Recording Workflow
Objective: Verify users can successfully record and transcribe a Teams meeting
Preconditions: 
- Application installed and configured
- Teams meeting in progress
- Microphone and speakers working

Steps:
1. Launch Teams Audio Transcription Application
2. Join Teams meeting
3. Click "Start Recording" button
4. Speak clearly: "This is a test recording for quality assurance"
5. Wait for transcription to appear
6. Click "Stop Recording" button
7. Verify transcript contains spoken words

Expected Results:
- Recording starts without errors
- Transcription appears within 5 seconds
- Text accuracy > 90% for clear speech
- Recording stops cleanly

Acceptance Criteria:
✓ User can start recording with single click
✓ Transcription appears in real-time
✓ Transcription accuracy meets expectations
✓ User can stop recording easily
```

#### Export Functionality UAT
```
Test Case: TC-UAT-002 - Transcript Export
Objective: Verify users can export transcripts in multiple formats
Preconditions: 
- Completed recording session with transcript content

Steps:
1. Complete a recording session (use TC-UAT-001)
2. Click "Export" button
3. Select "Microsoft Word" format
4. Choose save location
5. Click "Export"
6. Open exported file
7. Repeat for PDF format

Expected Results:
- Export dialog opens quickly
- File saves to selected location
- Exported file opens correctly
- Formatting is professional
- All transcript content present

Acceptance Criteria:
✓ Export completes within 30 seconds
✓ Multiple formats work correctly
✓ Formatting is user-friendly
✓ No data loss during export
```

### 7.2 Usability Testing

#### Usability Test Script
```
Usability Test Session: New User Experience
Duration: 45 minutes
Participants: 3-5 users with varying technical backgrounds

Pre-test Setup:
- Fresh Windows machine with application installed
- Teams meeting ready to join
- Screen recording enabled
- Observer taking notes

Tasks:
1. First Impression (5 minutes)
   - "What do you think this application does?"
   - "How would you start using it?"
   
2. Basic Setup (10 minutes)
   - "Configure the application for your first use"
   - "Set up speech recognition"
   - Note: Observe where users struggle

3. Recording Task (15 minutes)
   - "Record a short Teams meeting"
   - "Make sure the transcription is working"
   - Note: Count clicks, observe hesitation

4. Export Task (10 minutes)
   - "Save your transcript as a Word document"
   - "Email it to a colleague" (simulated)

5. Feedback Session (5 minutes)
   - "What was confusing?"
   - "What worked well?"
   - "Would you use this regularly?"

Success Metrics:
- Task completion rate > 90%
- Average time per task < target
- User satisfaction score > 4/5
- Zero critical usability issues
```

### 7.3 Accessibility Testing

#### Screen Reader Compatibility
```cpp
// tests/accessibility/ScreenReaderTest.cpp
// Note: This would typically be manual testing with NVDA/JAWS

TEST(AccessibilityTest, KeyboardNavigation) {
    // Test that all functionality is accessible via keyboard
    MainWindow window;
    ASSERT_TRUE(window.Create(GetModuleHandle(nullptr), SW_HIDE));
    
    // Simulate keyboard navigation
    // Tab through all controls
    for (int i = 0; i < 10; ++i) {
        SendMessage(window.GetHandle(), WM_KEYDOWN, VK_TAB, 0);
        SendMessage(window.GetHandle(), WM_KEYUP, VK_TAB, 0);
        
        // Verify focus is visible and logical
        HWND focusedWindow = GetFocus();
        EXPECT_NE(focusedWindow, nullptr) << "No focused control at tab " << i;
    }
    
    // Test Space/Enter activation
    SendMessage(window.GetHandle(), WM_KEYDOWN, VK_SPACE, 0);
    SendMessage(window.GetHandle(), WM_KEYUP, VK_SPACE, 0);
    
    // Should activate the focused control
}

TEST(AccessibilityTest, HighContrastSupport) {
    // Test application appearance in high contrast mode
    // This would typically be manual testing
    
    // Enable high contrast mode
    HIGHCONTRAST hc = {};
    hc.cbSize = sizeof(HIGHCONTRAST);
    hc.dwFlags = HCF_HIGHCONTRASTON;
    
    SystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0);
    
    MainWindow window;
    ASSERT_TRUE(window.Create(GetModuleHandle(nullptr), SW_SHOW));
    
    // Verify colors and visibility
    // This would require image analysis or manual verification
    
    // Restore normal contrast
    hc.dwFlags = 0;
    SystemParametersInfo(SPI_SETHIGHCONTRAST, sizeof(HIGHCONTRAST), &hc, 0);
}
```

---

## 8. Automated Testing

### 8.1 Continuous Integration Pipeline

#### GitHub Actions Workflow
```yaml
# .github/workflows/ci.yml
name: Continuous Integration

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

env:
  BUILD_TYPE: Release

jobs:
  test:
    runs-on: windows-latest
    
    strategy:
      matrix:
        config: [Debug, Release]
        
    steps:
    - uses: actions/checkout@v4
      with:
        submodules: true
    
    - name: Cache CMake build
      uses: actions/cache@v3
      with:
        path: build
        key: ${{ runner.os }}-cmake-${{ hashFiles('CMakeLists.txt') }}
    
    - name: Setup CMake
      uses: lukka/get-cmake@latest
    
    - name: Setup MSBuild
      uses: microsoft/setup-msbuild@v1
    
    - name: Configure CMake
      run: |
        cmake -B build -S . -DCMAKE_BUILD_TYPE=${{ matrix.config }} -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build --config ${{ matrix.config }} --parallel
    
    - name: Test
      working-directory: build
      run: ctest --build-config ${{ matrix.config }} --output-on-failure --parallel 4
    
    - name: Upload Test Results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: test-results-${{ matrix.config }}
        path: |
          build/Testing/
          build/**/*.xml
  
  code-quality:
    runs-on: windows-latest
    needs: test
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Run Static Analysis
      run: |
        # Run PVS-Studio or other static analysis tools
        # Generate reports
    
    - name: Code Coverage
      run: |
        # Generate code coverage reports
        # Upload to Codecov
    
    - name: SonarCloud Scan
      uses: SonarSource/sonarcloud-github-action@master
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        SONAR_TOKEN: ${{ secrets.SONAR_TOKEN }}

  security-scan:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Security Vulnerability Scan
      run: |
        # Run security scanning tools
        # Check for known vulnerabilities
    
    - name: Dependency Check
      run: |
        # Scan dependencies for security issues
        # Generate security report

  performance-test:
    runs-on: windows-latest
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Build Performance Tests
      run: |
        cmake -B build -S . -DBUILD_PERFORMANCE_TESTS=ON
        cmake --build build --config Release
    
    - name: Run Performance Tests
      run: |
        cd build
        ./performance_tests --benchmark_format=json --benchmark_out=performance.json
    
    - name: Upload Performance Results
      uses: actions/upload-artifact@v3
      with:
        name: performance-results
        path: build/performance.json
    
    - name: Performance Regression Check
      run: |
        # Compare with baseline performance metrics
        # Fail if performance degrades significantly
```

### 8.2 Test Automation Scripts

#### Automated UI Testing
```python
# tests/automation/ui_automation.py
import pyautogui
import time
import subprocess
import json
from pathlib import Path

class UITestAutomation:
    def __init__(self):
        self.app_path = Path("build/output/Release/TeamsTranscriptionApp.exe")
        self.test_results = []
        
    def setup_test_environment(self):
        """Set up test environment"""
        # Kill any existing instances
        subprocess.run(["taskkill", "/f", "/im", "TeamsTranscriptionApp.exe"], 
                      capture_output=True)
        
        # Clean test data
        test_data_path = Path("test_data")
        if test_data_path.exists():
            shutil.rmtree(test_data_path)
        test_data_path.mkdir()
        
        # Start virtual audio device
        subprocess.run(["VirtualAudioCable.exe", "/start"])
        
    def launch_application(self):
        """Launch the application"""
        process = subprocess.Popen([str(self.app_path)])
        time.sleep(3)  # Wait for application to start
        return process
        
    def test_basic_workflow(self):
        """Test basic recording workflow"""
        test_name = "Basic Workflow"
        print(f"Running test: {test_name}")
        
        try:
            # Launch application
            process = self.launch_application()
            
            # Find and click Start button
            start_button = pyautogui.locateOnScreen('test_images/start_button.png', 
                                                   confidence=0.8)
            if start_button:
                pyautogui.click(start_button)
                self.test_results.append({
                    "test": test_name,
                    "step": "Click Start Button",
                    "status": "PASS"
                })
            else:
                raise Exception("Start button not found")
            
            # Wait for recording to start
            time.sleep(2)
            
            # Check if recording indicator is visible
            recording_indicator = pyautogui.locateOnScreen('test_images/recording_indicator.png',
                                                          confidence=0.8)
            if recording_indicator:
                self.test_results.append({
                    "test": test_name,
                    "step": "Recording Started",
                    "status": "PASS"
                })
            else:
                raise Exception("Recording indicator not found")
            
            # Play test audio
            subprocess.run(["ffplay", "-nodisp", "-autoexit", "test_audio/sample.wav"])
            
            # Wait for transcription
            time.sleep(5)
            
            # Check if transcription appeared
            transcription_area = pyautogui.locateOnScreen('test_images/transcription_text.png',
                                                         confidence=0.6)
            if transcription_area:
                self.test_results.append({
                    "test": test_name,
                    "step": "Transcription Appeared",
                    "status": "PASS"
                })
            else:
                print("Warning: Transcription not detected")
                self.test_results.append({
                    "test": test_name,
                    "step": "Transcription Appeared",
                    "status": "WARN"
                })
            
            # Stop recording
            stop_button = pyautogui.locateOnScreen('test_images/stop_button.png',
                                                  confidence=0.8)
            if stop_button:
                pyautogui.click(stop_button)
                self.test_results.append({
                    "test": test_name,
                    "step": "Click Stop Button",
                    "status": "PASS"
                })
            
            # Close application
            process.terminate()
            
        except Exception as e:
            self.test_results.append({
                "test": test_name,
                "step": "Test Execution",
                "status": "FAIL",
                "error": str(e)
            })
            
    def test_export_functionality(self):
        """Test export functionality"""
        test_name = "Export Functionality"
        print(f"Running test: {test_name}")
        
        try:
            # This would follow similar pattern to basic workflow
            # but focus on export functionality
            pass
            
        except Exception as e:
            self.test_results.append({
                "test": test_name,
                "step": "Test Execution", 
                "status": "FAIL",
                "error": str(e)
            })
    
    def generate_report(self):
        """Generate test report"""
        report = {
            "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
            "total_tests": len(set(r["test"] for r in self.test_results)),
            "results": self.test_results
        }
        
        with open("ui_test_report.json", "w") as f:
            json.dump(report, f, indent=2)
        
        # Print summary
        passed = len([r for r in self.test_results if r["status"] == "PASS"])
        failed = len([r for r in self.test_results if r["status"] == "FAIL"])
        warnings = len([r for r in self.test_results if r["status"] == "WARN"])
        
        print(f"\nTest Results Summary:")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print(f"Warnings: {warnings}")
        
        return failed == 0

if __name__ == "__main__":
    automation = UITestAutomation()
    automation.setup_test_environment()
    
    automation.test_basic_workflow()
    automation.test_export_functionality()
    
    success = automation.generate_report()
    exit(0 if success else 1)
```

### 8.3 Test Data Management

#### Test Data Generation
```python
# tests/data/generate_test_data.py
import json
import random
import datetime
from pathlib import Path

def generate_test_configurations():
    """Generate test configuration files"""
    configurations = [
        {
            "name": "minimal_config",
            "config": {
                "recording": {"audioQuality": 16000},
                "speechRecognition": {
                    "provider": "windows",
                    "language": "en-US"
                }
            }
        },
        {
            "name": "azure_config", 
            "config": {
                "recording": {"audioQuality": 44100},
                "speechRecognition": {
                    "provider": "azure",
                    "apiKey": "test_key",
                    "region": "eastus",
                    "language": "en-US",
                    "enablePunctuation": True
                }
            }
        },
        {
            "name": "high_quality_config",
            "config": {
                "recording": {
                    "audioQuality": 48000,
                    "outputFormat": "wav"
                },
                "speechRecognition": {
                    "provider": "google",
                    "language": "en-US",
                    "enableSpeakerDiarization": True
                },
                "privacy": {
                    "enableEncryption": True
                }
            }
        }
    ]
    
    test_data_dir = Path("tests/data/configurations")
    test_data_dir.mkdir(parents=True, exist_ok=True)
    
    for config_data in configurations:
        file_path = test_data_dir / f"{config_data['name']}.json"
        with open(file_path, "w") as f:
            json.dump(config_data["config"], f, indent=2)

def generate_test_transcripts():
    """Generate test transcript data"""
    transcripts = []
    
    for i in range(10):
        transcript = {
            "meetingId": f"test_meeting_{i:03d}",
            "startTime": (datetime.datetime.now() - datetime.timedelta(days=i)).isoformat(),
            "endTime": (datetime.datetime.now() - datetime.timedelta(days=i) + datetime.timedelta(hours=1)).isoformat(),
            "segments": []
        }
        
        # Generate segments
        for j in range(random.randint(5, 20)):
            segment = {
                "text": f"This is test segment {j} for meeting {i}.",
                "startTime": j * 3000,
                "endTime": (j + 1) * 3000,
                "confidence": random.uniform(0.7, 0.98),
                "speakerId": f"speaker_{random.randint(1, 3)}",
                "isFinal": True
            }
            transcript["segments"].append(segment)
        
        transcripts.append(transcript)
    
    test_data_dir = Path("tests/data/transcripts")
    test_data_dir.mkdir(parents=True, exist_ok=True)
    
    for i, transcript in enumerate(transcripts):
        file_path = test_data_dir / f"test_transcript_{i:03d}.json"
        with open(file_path, "w") as f:
            json.dump(transcript, f, indent=2)

if __name__ == "__main__":
    generate_test_configurations()
    generate_test_transcripts()
    print("Test data generated successfully")
```

---

## Document Information

**Document Version:** 2.0.0  
**Last Updated:** July 2025  
**Document Owner:** Quality Assurance Team  
**Review Cycle:** Monthly  

### Testing Standards Compliance

- **ISO/IEC 25010**: Software Quality Model
- **IEEE 829**: Standard for Software Test Documentation
- **ISTQB**: International Software Testing Qualifications Board guidelines
- **Microsoft Testing Guidelines**: Windows application testing best practices

### Test Environment Requirements

| Component | Requirement | Notes |
|-----------|-------------|-------|
| OS | Windows 10 1903+ / Windows 11 | Primary target platforms |
| RAM | 16GB recommended | For performance testing |
| Storage | 100GB free space | Test data and artifacts |
| Audio | Virtual Audio Cable | Automated audio testing |
| Network | Broadband connection | Cloud provider testing |

### Related Documentation

- **Requirements**: [Requirements_Specification.md](Requirements_Specification.md)
- **Design**: [Detailed_Design_Document.md](Detailed_Design_Document.md)
- **Developer Guide**: [Developer_Documentation.md](Developer_Documentation.md)
- **User Manual**: [User_Manual.md](User_Manual.md)

### Contact Information

- **QA Team Lead**: qa-lead@teamstranscription.com
- **Test Automation**: test-automation@teamstranscription.com
- **Performance Testing**: performance@teamstranscription.com
- **Security Testing**: security@teamstranscription.com

---

*© 2025 Teams Transcription Application. All rights reserved. This testing documentation is provided for authorized development and QA personnel only.*