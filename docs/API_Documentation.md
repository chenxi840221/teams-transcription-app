# Teams Audio Transcription Application
## API Documentation

**Version:** 2.0.0  
**Date:** July 2025  
**Document Type:** API Reference  

---

## Table of Contents

1. [Overview](#1-overview)
2. [Core APIs](#2-core-apis)
3. [Speech Recognition APIs](#3-speech-recognition-apis)
4. [Configuration APIs](#4-configuration-apis)
5. [Export APIs](#5-export-apis)
6. [Error Handling](#6-error-handling)
7. [Examples and Usage](#7-examples-and-usage)
8. [Integration Guidelines](#8-integration-guidelines)

---

## 1. Overview

### 1.1 API Philosophy

The Teams Audio Transcription Application provides a component-based architecture with well-defined interfaces for:
- Audio capture and processing
- Speech recognition with multiple providers
- Configuration management
- Export functionality
- User interface components

### 1.2 Interface Design Principles

- **Consistency**: All interfaces follow similar patterns
- **Extensibility**: Easy to add new providers and formats
- **Error Safety**: Comprehensive error handling and reporting
- **Performance**: Designed for real-time audio processing
- **Thread Safety**: Safe for multi-threaded applications

### 1.3 API Conventions

#### Return Values
- **HRESULT**: Windows APIs and system-level operations
- **bool**: Simple success/failure operations
- **std::optional**: Operations that may not return a value
- **Exceptions**: Internal errors and invalid parameters

#### Parameter Patterns
- **const references**: For input parameters that won't be modified
- **Pointers**: For optional parameters or COM interfaces
- **Callbacks**: Using std::function for event handling
- **Smart pointers**: For resource management

---

## 2. Core APIs

### 2.1 AudioCapture API

#### Class Overview
```cpp
class AudioCapture {
public:
    enum class AudioQuality {
        Low = 8000,     ///< 8kHz - Voice quality
        Medium = 16000, ///< 16kHz - Standard quality (recommended)
        High = 44100    ///< 44.1kHz - CD quality
    };
    
    struct AudioFormat {
        UINT32 sampleRate;      ///< Samples per second
        UINT16 channels;        ///< Number of audio channels (1=mono, 2=stereo)
        UINT16 bitsPerSample;   ///< Bit depth (typically 16)
        UINT32 bytesPerSecond;  ///< Calculated data rate
    };
    
    struct CaptureStats {
        UINT64 totalFramesCaptured;  ///< Total audio frames processed
        UINT64 totalBytesProcessed;  ///< Total bytes of audio data
        double captureTimeSeconds;   ///< Total capture time in seconds
        UINT32 bufferUnderruns;      ///< Number of buffer underruns
    };
    
    using AudioDataCallback = std::function<void(const std::vector<BYTE>&, const AudioFormat&)>;
};
```

#### Core Methods

##### Initialize
```cpp
/**
 * @brief Initialize the audio capture system
 * @param quality Desired audio quality level
 * @return S_OK on success, error HRESULT on failure
 * @note Must be called before StartCapture()
 */
HRESULT Initialize(AudioQuality quality = AudioQuality::Medium);
```

**Usage:**
```cpp
AudioCapture capture;
HRESULT hr = capture.Initialize(AudioCapture::AudioQuality::High);
if (FAILED(hr)) {
    // Handle initialization error
    std::cerr << "Failed to initialize audio capture: 0x" << std::hex << hr << std::endl;
}
```

##### StartCapture / StopCapture
```cpp
/**
 * @brief Start audio capture
 * @return S_OK on success, error HRESULT on failure
 * @note Initialize() must be called first
 */
HRESULT StartCapture();

/**
 * @brief Stop audio capture
 * @note Safe to call multiple times
 */
void StopCapture();

/**
 * @brief Check if audio capture is active
 * @return true if capturing, false otherwise
 */
bool IsCapturing() const;
```

**Usage:**
```cpp
// Start capture
HRESULT hr = capture.StartCapture();
if (SUCCEEDED(hr)) {
    std::cout << "Audio capture started" << std::endl;
    
    // Do work...
    
    // Stop when done
    capture.StopCapture();
}
```

##### Audio Data Callback
```cpp
/**
 * @brief Set callback for receiving audio data
 * @param callback Function to call with audio data
 * @note Callback is called from audio thread
 */
void SetAudioDataCallback(AudioDataCallback callback);
```

**Usage:**
```cpp
capture.SetAudioDataCallback([](const std::vector<BYTE>& audioData, 
                               const AudioCapture::AudioFormat& format) {
    // Process audio data (called from audio thread)
    std::cout << "Received " << audioData.size() << " bytes of audio data" << std::endl;
    std::cout << "Format: " << format.sampleRate << "Hz, " 
              << format.channels << " channels" << std::endl;
});
```

##### Statistics and Monitoring
```cpp
/**
 * @brief Get current audio format
 * @return Audio format specification
 */
AudioFormat GetAudioFormat() const;

/**
 * @brief Get capture statistics
 * @return Current capture statistics
 */
CaptureStats GetCaptureStats() const;
```

**Usage:**
```cpp
// Monitor capture performance
auto stats = capture.GetCaptureStats();
std::cout << "Captured " << stats.totalFramesCaptured << " frames in " 
          << stats.captureTimeSeconds << " seconds" << std::endl;

if (stats.bufferUnderruns > 0) {
    std::cout << "Warning: " << stats.bufferUnderruns << " buffer underruns detected" << std::endl;
}
```

#### Error Codes

| HRESULT | Description | Resolution |
|---------|-------------|------------|
| `S_OK` | Success | None |
| `E_INVALIDARG` | Invalid parameter | Check parameter values |
| `AUDCLNT_E_DEVICE_INVALIDATED` | Audio device removed | Reinitialize with new device |
| `AUDCLNT_E_SERVICE_NOT_RUNNING` | Audio service not running | Start Windows Audio service |
| `E_OUTOFMEMORY` | Insufficient memory | Reduce audio quality or close other applications |

### 2.2 MainWindow API

#### Class Overview
```cpp
class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    /**
     * @brief Create and display the main window
     * @param hInstance Application instance handle
     * @param nCmdShow Window display state
     * @return true on success, false on failure
     */
    bool Create(HINSTANCE hInstance, int nCmdShow);
    
    /**
     * @brief Run the message loop
     * @return Application exit code
     */
    int MessageLoop();
    
    /**
     * @brief Check if recording is active
     * @return true if recording, false otherwise
     */
    bool IsRecording() const { return isRecording.load(); }
    
    /**
     * @brief Get current transcript data
     * @return Current meeting transcript
     */
    MeetingTranscript GetCurrentTranscript() const;

private:
    std::atomic<bool> isRecording;
    std::atomic<bool> isPaused;
    std::unique_ptr<AudioCapture> audioCapture;
    std::unique_ptr<SpeechRecognition> speechRecognition;
    std::unique_ptr<ProcessMonitor> processMonitor;
    std::unique_ptr<ConfigManager> configManager;
};
```

#### Window Management

##### Create Window
```cpp
bool Create(HINSTANCE hInstance, int nCmdShow);
```

**Usage:**
```cpp
MainWindow window;
if (window.Create(GetModuleHandle(nullptr), SW_SHOW)) {
    int exitCode = window.MessageLoop();
    return exitCode;
} else {
    std::cerr << "Failed to create main window" << std::endl;
    return -1;
}
```

#### Recording Control

##### Start/Stop Recording
```cpp
/**
 * @brief Start recording session
 * @note Will prompt for consent if configured
 */
void StartRecording();

/**
 * @brief Stop current recording session
 */
void StopRecording();

/**
 * @brief Pause/resume recording
 */
void TogglePause();
```

**Usage:**
```cpp
// Start recording (programmatically)
window.StartRecording();

// Check status
if (window.IsRecording()) {
    std::cout << "Recording is active" << std::endl;
}

// Stop when done
window.StopRecording();
```

---

## 3. Speech Recognition APIs

### 3.1 SpeechRecognition API

#### Class Overview
```cpp
class SpeechRecognition {
public:
    enum class Provider {
        Azure,      ///< Azure Cognitive Services
        Google,     ///< Google Cloud Speech-to-Text
        OpenAI,     ///< OpenAI Whisper API
        Amazon,     ///< Amazon Transcribe
        Windows     ///< Windows Speech Recognition (offline)
    };
    
    struct SpeechConfig {
        Provider provider;              ///< Speech recognition provider
        std::string apiKey;            ///< API key for cloud providers
        std::string region;            ///< Service region (Azure, AWS)
        std::string language;          ///< Language code (e.g., "en-US")
        bool enablePunctuation;        ///< Add punctuation to results
        bool enableSpeakerDiarization; ///< Identify different speakers
    };
    
    using TranscriptionCallback = std::function<void(const std::string& text, double confidence)>;
};
```

#### Core Methods

##### Initialize
```cpp
/**
 * @brief Initialize speech recognition with specified configuration
 * @param config Speech recognition configuration
 * @return true on success, false on failure
 */
bool Initialize(const SpeechConfig& config);

/**
 * @brief Check if speech recognition is initialized
 * @return true if initialized, false otherwise
 */
bool IsInitialized() const;
```

**Usage:**
```cpp
SpeechRecognition speech;
SpeechRecognition::SpeechConfig config;

// Configure for Azure
config.provider = SpeechRecognition::Provider::Azure;
config.apiKey = "your_azure_key";
config.region = "eastus";
config.language = "en-US";
config.enablePunctuation = true;
config.enableSpeakerDiarization = false;

if (speech.Initialize(config)) {
    std::cout << "Speech recognition initialized successfully" << std::endl;
} else {
    std::cerr << "Failed to initialize speech recognition" << std::endl;
}
```

##### Process Audio Data
```cpp
/**
 * @brief Process audio data for speech recognition
 * @param audioData Raw audio data bytes
 * @param format Audio format specification
 * @note Called from audio capture callback
 */
void ProcessAudioData(const std::vector<BYTE>& audioData, 
                     const AudioCapture::AudioFormat& format);
```

**Usage:**
```cpp
// Set up audio capture callback to feed speech recognition
audioCapture.SetAudioDataCallback([&speech](const std::vector<BYTE>& audioData,
                                            const AudioCapture::AudioFormat& format) {
    speech.ProcessAudioData(audioData, format);
});
```

##### Transcription Callback
```cpp
/**
 * @brief Set callback for receiving transcription results
 * @param callback Function to call with transcription text and confidence
 */
void SetTranscriptionCallback(TranscriptionCallback callback);
```

**Usage:**
```cpp
speech.SetTranscriptionCallback([](const std::string& text, double confidence) {
    std::cout << "Transcription (" << std::fixed << std::setprecision(2) 
              << confidence * 100 << "%): " << text << std::endl;
    
    if (confidence < 0.7) {
        std::cout << "Warning: Low confidence transcription" << std::endl;
    }
});
```

### 3.2 Provider-Specific Configuration

#### Azure Cognitive Services
```cpp
// Azure configuration example
SpeechRecognition::SpeechConfig azureConfig;
azureConfig.provider = SpeechRecognition::Provider::Azure;
azureConfig.apiKey = "your_azure_subscription_key";
azureConfig.region = "eastus";  // or your preferred region
azureConfig.language = "en-US";
azureConfig.enablePunctuation = true;
azureConfig.enableSpeakerDiarization = true;  // Premium feature
```

**Required Setup:**
1. Create Azure Cognitive Services resource
2. Get subscription key and region
3. Ensure network connectivity to `*.cognitiveservices.azure.com`

#### Google Cloud Speech-to-Text
```cpp
// Google configuration example
SpeechRecognition::SpeechConfig googleConfig;
googleConfig.provider = SpeechRecognition::Provider::Google;
googleConfig.apiKey = "your_google_api_key";  // or use service account
googleConfig.language = "en-US";
googleConfig.enablePunctuation = true;
googleConfig.enableSpeakerDiarization = false;  // Configure separately
```

**Required Setup:**
1. Enable Speech-to-Text API in Google Cloud Console
2. Create API key or service account
3. Ensure network connectivity to `*.googleapis.com`

#### Windows Speech Recognition
```cpp
// Windows configuration (no API key required)
SpeechRecognition::SpeechConfig windowsConfig;
windowsConfig.provider = SpeechRecognition::Provider::Windows;
windowsConfig.language = "en-US";
windowsConfig.enablePunctuation = false;  // Limited support
windowsConfig.enableSpeakerDiarization = false;  // Not supported
```

**Benefits:**
- No API key required
- Works offline
- No usage costs
- Privacy-focused (local processing)

**Limitations:**
- Lower accuracy than cloud providers
- Limited language support
- No speaker diarization
- Basic punctuation

### 3.3 Custom Provider Interface

#### ISpeechProvider Interface
```cpp
class ISpeechProvider {
public:
    virtual ~ISpeechProvider() = default;
    
    /**
     * @brief Initialize the speech provider
     * @param config Provider configuration
     * @return true on success, false on failure
     */
    virtual bool Initialize(const SpeechConfig& config) = 0;
    
    /**
     * @brief Process audio data for recognition
     * @param audioData Raw audio bytes
     * @param format Audio format specification
     */
    virtual void ProcessAudioData(const std::vector<BYTE>& audioData,
                                 const AudioCapture::AudioFormat& format) = 0;
    
    /**
     * @brief Set transcription result callback
     * @param callback Function to receive results
     */
    virtual void SetTranscriptionCallback(TranscriptionCallback callback) = 0;
    
    /**
     * @brief Check if provider is initialized
     * @return true if ready for processing
     */
    virtual bool IsInitialized() const = 0;
};
```

#### Implementing Custom Provider
```cpp
class CustomSpeechProvider : public ISpeechProvider {
public:
    bool Initialize(const SpeechConfig& config) override {
        // Initialize your custom provider
        config_ = config;
        
        // Validate configuration
        if (config_.apiKey.empty()) {
            return false;
        }
        
        // Set up connections, authentication, etc.
        httpClient_.SetAuthToken(config_.apiKey);
        
        initialized_ = true;
        return true;
    }
    
    void ProcessAudioData(const std::vector<BYTE>& audioData,
                         const AudioCapture::AudioFormat& format) override {
        if (!initialized_) return;
        
        // Buffer audio data
        audioBuffer_.Append(audioData);
        
        // Process when threshold reached
        if (audioBuffer_.GetSizeBytes() >= processingThreshold_) {
            SendToRecognitionService();
        }
    }
    
    void SetTranscriptionCallback(TranscriptionCallback callback) override {
        transcriptionCallback_ = callback;
    }
    
    bool IsInitialized() const override {
        return initialized_;
    }

private:
    bool initialized_ = false;
    SpeechConfig config_;
    TranscriptionCallback transcriptionCallback_;
    AudioBuffer audioBuffer_;
    HttpClient httpClient_;
    size_t processingThreshold_ = 32000;  // ~1 second at 16kHz
    
    void SendToRecognitionService() {
        // Convert audio to required format
        std::string audioData = audioBuffer_.ToBase64();
        
        // Create API request
        nlohmann::json request;
        request["audio"] = audioData;
        request["format"] = "wav";
        request["language"] = config_.language;
        
        // Send to service
        auto response = httpClient_.Post("/recognize", request.dump());
        
        if (response.status == 200) {
            ProcessResponse(response.body);
        }
        
        audioBuffer_.Clear();
    }
    
    void ProcessResponse(const std::string& responseJson) {
        try {
            auto json = nlohmann::json::parse(responseJson);
            
            if (json.contains("transcript")) {
                std::string text = json["transcript"];
                double confidence = json.value("confidence", 0.0);
                
                if (transcriptionCallback_) {
                    transcriptionCallback_(text, confidence);
                }
            }
        } catch (const std::exception& e) {
            // Handle parsing error
            std::cerr << "Error parsing response: " << e.what() << std::endl;
        }
    }
};
```

---

## 4. Configuration APIs

### 4.1 ConfigManager API

#### Class Overview
```cpp
class ConfigManager {
public:
    struct AppConfig {
        // Recording settings
        bool autoStartRecording;
        std::string outputFormat;
        std::string outputDirectory;
        int audioQuality;
        
        // Speech recognition configuration
        SpeechRecognition::SpeechConfig speechConfig;
        
        // UI settings
        bool minimizeToTray;
        bool showNotifications;
        std::string theme;
        
        // Privacy settings
        bool requireConsent;
        int dataRetentionDays;
        bool enableEncryption;
        
        // Export settings
        std::vector<std::string> exportFormats;
        bool autoExport;
        std::string exportTemplate;
    };
};
```

#### Core Methods

##### Load/Save Configuration
```cpp
/**
 * @brief Load configuration from file
 * @param configPath Path to configuration file (empty for default)
 * @return true on success, false on failure
 */
bool LoadConfig(const std::string& configPath = "");

/**
 * @brief Save configuration to file
 * @param configPath Path to configuration file (empty for default)
 * @return true on success, false on failure
 */
bool SaveConfig(const std::string& configPath = "");

/**
 * @brief Get current configuration (read-only)
 * @return Const reference to configuration
 */
const AppConfig& GetConfig() const;

/**
 * @brief Get current configuration (modifiable)
 * @return Reference to configuration
 */
AppConfig& GetConfig();
```

**Usage:**
```cpp
ConfigManager configManager;

// Load configuration
if (configManager.LoadConfig()) {
    const auto& config = configManager.GetConfig();
    std::cout << "Loaded configuration:" << std::endl;
    std::cout << "  Audio quality: " << config.audioQuality << std::endl;
    std::cout << "  Speech provider: " << static_cast<int>(config.speechConfig.provider) << std::endl;
} else {
    std::cerr << "Failed to load configuration, using defaults" << std::endl;
}

// Modify configuration
auto& config = configManager.GetConfig();
config.audioQuality = 44100;
config.speechConfig.language = "en-GB";

// Save changes
if (!configManager.SaveConfig()) {
    std::cerr << "Failed to save configuration" << std::endl;
}
```

##### Speech Configuration
```cpp
/**
 * @brief Get speech recognition configuration
 * @return Current speech configuration
 */
SpeechRecognition::SpeechConfig GetSpeechConfig() const;

/**
 * @brief Update speech recognition configuration
 * @param speechConfig New speech configuration
 */
void SetSpeechConfig(const SpeechRecognition::SpeechConfig& speechConfig);
```

**Usage:**
```cpp
// Update speech configuration
SpeechRecognition::SpeechConfig speechConfig;
speechConfig.provider = SpeechRecognition::Provider::Google;
speechConfig.apiKey = "new_google_api_key";
speechConfig.language = "fr-FR";
speechConfig.enablePunctuation = true;

configManager.SetSpeechConfig(speechConfig);
configManager.SaveConfig();  // Persist changes
```

### 4.2 Configuration File Format

#### JSON Schema
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "recording": {
      "type": "object",
      "properties": {
        "autoStart": {"type": "boolean"},
        "outputFormat": {"type": "string", "enum": ["wav", "mp3"]},
        "outputDirectory": {"type": "string"},
        "audioQuality": {"type": "integer", "enum": [8000, 16000, 44100]}
      },
      "required": ["autoStart", "outputFormat", "audioQuality"]
    },
    "speechRecognition": {
      "type": "object",
      "properties": {
        "provider": {"type": "string", "enum": ["azure", "google", "openai", "amazon", "windows"]},
        "apiKey": {"type": "string"},
        "region": {"type": "string"},
        "language": {"type": "string"},
        "enablePunctuation": {"type": "boolean"},
        "enableSpeakerDiarization": {"type": "boolean"}
      },
      "required": ["provider", "language"]
    },
    "ui": {
      "type": "object",
      "properties": {
        "minimizeToTray": {"type": "boolean"},
        "showNotifications": {"type": "boolean"},
        "theme": {"type": "string", "enum": ["system", "light", "dark"]}
      }
    },
    "privacy": {
      "type": "object",
      "properties": {
        "requireConsent": {"type": "boolean"},
        "dataRetentionDays": {"type": "integer", "minimum": 1, "maximum": 365},
        "enableEncryption": {"type": "boolean"}
      }
    }
  }
}
```

#### Example Configuration
```json
{
  "recording": {
    "autoStart": false,
    "outputFormat": "wav",
    "outputDirectory": "./data/recordings",
    "audioQuality": 16000
  },
  "speechRecognition": {
    "provider": "azure",
    "apiKey": "your_api_key_here",
    "region": "eastus",
    "language": "en-US",
    "enablePunctuation": true,
    "enableSpeakerDiarization": true
  },
  "ui": {
    "minimizeToTray": true,
    "showNotifications": true,
    "theme": "system"
  },
  "privacy": {
    "requireConsent": true,
    "dataRetentionDays": 30,
    "enableEncryption": true
  },
  "export": {
    "formats": ["txt", "docx", "pdf"],
    "autoExport": false,
    "template": "meeting-transcript",
    "includeTimestamps": true,
    "includeSpeakerNames": true
  }
}
```

---

## 5. Export APIs

### 5.1 Export System Architecture

#### IExportFormat Interface
```cpp
class IExportFormat {
public:
    virtual ~IExportFormat() = default;
    
    /**
     * @brief Export transcript to specified format
     * @param transcript Meeting transcript data
     * @param outputPath Output file path
     * @return true on success, false on failure
     */
    virtual bool Export(const MeetingTranscript& transcript, 
                       const std::string& outputPath) = 0;
    
    /**
     * @brief Get file extension for this format
     * @return File extension (e.g., ".txt", ".docx")
     */
    virtual std::string GetFileExtension() const = 0;
    
    /**
     * @brief Get human-readable format name
     * @return Format name (e.g., "Plain Text", "Microsoft Word")
     */
    virtual std::string GetFormatName() const = 0;
    
    /**
     * @brief Check if format supports templates
     * @return true if templates are supported
     */
    virtual bool SupportsTemplate() const = 0;
};
```

#### ExportManager Class
```cpp
class ExportManager {
public:
    enum class Format {
        TXT,        ///< Plain text format
        DOCX,       ///< Microsoft Word document
        PDF,        ///< Portable Document Format
        JSON,       ///< JSON data format
        HTML,       ///< HTML web page
        Markdown    ///< Markdown format
    };
    
    /**
     * @brief Register a new export format
     * @param format Format enumeration value
     * @param exporter Format implementation
     * @return true on success, false if format already registered
     */
    bool RegisterFormat(Format format, std::unique_ptr<IExportFormat> exporter);
    
    /**
     * @brief Export transcript in specified format
     * @param transcript Meeting transcript data
     * @param format Desired export format
     * @param outputPath Output file path
     * @param templatePath Optional template file path
     * @return true on success, false on failure
     */
    bool Export(const MeetingTranscript& transcript, Format format, 
               const std::string& outputPath, const std::string& templatePath = "");
    
    /**
     * @brief Get list of supported export formats
     * @return Vector of supported formats
     */
    std::vector<Format> GetSupportedFormats() const;
    
    /**
     * @brief Get format name for display
     * @param format Format enumeration value
     * @return Human-readable format name
     */
    std::string GetFormatName(Format format) const;
    
    /**
     * @brief Get file extension for format
     * @param format Format enumeration value
     * @return File extension string
     */
    std::string GetFileExtension(Format format) const;
};
```

### 5.2 Data Structures

#### MeetingTranscript
```cpp
struct MeetingTranscript {
    std::string meetingId;                              ///< Unique meeting identifier
    std::chrono::system_clock::time_point startTime;   ///< Meeting start time
    std::chrono::system_clock::time_point endTime;     ///< Meeting end time
    std::vector<TranscriptionSegment> segments;        ///< Transcript segments
    TranscriptMetadata metadata;                        ///< Additional metadata
};

struct TranscriptionSegment {
    std::string text;                                   ///< Transcribed text
    std::chrono::milliseconds startTime;               ///< Segment start offset
    std::chrono::milliseconds endTime;                 ///< Segment end offset
    double confidence;                                  ///< Recognition confidence (0.0-1.0)
    std::string speakerId;                             ///< Speaker identifier
    bool isFinal;                                      ///< Final vs. interim result
};

struct TranscriptMetadata {
    std::string applicationVersion;                     ///< App version used
    std::string speechProvider;                        ///< Speech recognition provider
    std::string language;                              ///< Recognition language
    AudioCapture::AudioFormat audioFormat;            ///< Audio format used
    size_t totalAudioBytes;                           ///< Total audio data processed
    double averageConfidence;                         ///< Average confidence score
};
```

### 5.3 Export Usage Examples

#### Basic Export
```cpp
ExportManager exportManager;

// Load transcript data
MeetingTranscript transcript = LoadTranscriptFromFile("meeting_001.json");

// Export to plain text
bool success = exportManager.Export(transcript, 
                                   ExportManager::Format::TXT,
                                   "meeting_001.txt");

if (success) {
    std::cout << "Transcript exported successfully" << std::endl;
} else {
    std::cerr << "Export failed" << std::endl;
}
```

#### Multiple Format Export
```cpp
// Export to multiple formats
std::vector<ExportManager::Format> formats = {
    ExportManager::Format::TXT,
    ExportManager::Format::DOCX,
    ExportManager::Format::PDF
};

for (auto format : formats) {
    std::string filename = "meeting_001" + exportManager.GetFileExtension(format);
    
    if (exportManager.Export(transcript, format, filename)) {
        std::cout << "Exported: " << filename << std::endl;
    } else {
        std::cerr << "Failed to export: " << filename << std::endl;
    }
}
```

#### Template-Based Export
```cpp
// Export with custom template
bool success = exportManager.Export(transcript,
                                   ExportManager::Format::DOCX,
                                   "formal_meeting.docx",
                                   "templates/formal_meeting_template.docx");
```

### 5.4 Custom Export Format Implementation

#### Example: CSV Exporter
```cpp
class CSVExporter : public IExportFormat {
public:
    bool Export(const MeetingTranscript& transcript, 
               const std::string& outputPath) override {
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            return false;
        }
        
        // Write CSV header
        file << "Timestamp,Speaker,Text,Confidence\n";
        
        // Write segments
        for (const auto& segment : transcript.segments) {
            file << FormatTimestamp(segment.startTime) << ","
                 << EscapeCSV(segment.speakerId) << ","
                 << EscapeCSV(segment.text) << ","
                 << std::fixed << std::setprecision(3) << segment.confidence << "\n";
        }
        
        return true;
    }
    
    std::string GetFileExtension() const override {
        return ".csv";
    }
    
    std::string GetFormatName() const override {
        return "Comma-Separated Values";
    }
    
    bool SupportsTemplate() const override {
        return false;
    }

private:
    std::string FormatTimestamp(std::chrono::milliseconds timestamp) {
        // Convert to HH:MM:SS format
        auto seconds = timestamp.count() / 1000;
        auto hours = seconds / 3600;
        auto minutes = (seconds % 3600) / 60;
        auto secs = seconds % 60;
        
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setfill('0') << std::setw(2) << minutes << ":"
            << std::setfill('0') << std::setw(2) << secs;
        return oss.str();
    }
    
    std::string EscapeCSV(const std::string& text) {
        std::string escaped = text;
        
        // Escape quotes by doubling them
        size_t pos = 0;
        while ((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.insert(pos, "\"");
            pos += 2;
        }
        
        // Quote if contains comma, quote, or newline
        if (escaped.find(',') != std::string::npos ||
            escaped.find('"') != std::string::npos ||
            escaped.find('\n') != std::string::npos) {
            escaped = "\"" + escaped + "\"";
        }
        
        return escaped;
    }
};

// Register the custom exporter
exportManager.RegisterFormat(ExportManager::Format::CSV, 
                            std::make_unique<CSVExporter>());
```

---

## 6. Error Handling

### 6.1 Error Categories

#### System Errors (HRESULT)
Used for Windows API calls and system-level operations:

```cpp
// Audio system errors
AUDCLNT_E_DEVICE_INVALIDATED   // Audio device removed
AUDCLNT_E_SERVICE_NOT_RUNNING  // Windows Audio service stopped
AUDCLNT_E_BUFFER_TOO_LARGE     // Audio buffer size too large
AUDCLNT_E_BUFFER_SIZE_ERROR    // Invalid buffer size

// COM errors
E_INVALIDARG                   // Invalid parameter
E_OUTOFMEMORY                 // Insufficient memory
E_FAIL                        // General failure
E_ACCESSDENIED                // Access denied

// Custom application errors
MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1001)  // Speech provider error
MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x1002)  // Configuration error
```

#### Application Exceptions
Used for internal errors and invalid parameters:

```cpp
class AudioCaptureException : public std::runtime_error {
public:
    explicit AudioCaptureException(const std::string& message, HRESULT hr = E_FAIL)
        : std::runtime_error(message), errorCode_(hr) {}
    
    HRESULT GetErrorCode() const { return errorCode_; }

private:
    HRESULT errorCode_;
};

class SpeechRecognitionException : public std::runtime_error {
public:
    enum class ErrorType {
        InvalidConfiguration,
        NetworkError,
        AuthenticationFailure,
        QuotaExceeded,
        ServiceUnavailable
    };
    
    SpeechRecognitionException(ErrorType type, const std::string& message)
        : std::runtime_error(message), errorType_(type) {}
    
    ErrorType GetErrorType() const { return errorType_; }

private:
    ErrorType errorType_;
};
```

### 6.2 Error Handling Patterns

#### HRESULT Pattern
```cpp
HRESULT AudioCapture::StartCapture() {
    if (IsCapturing()) {
        return S_OK;  // Already capturing
    }
    
    if (!audioClient) {
        return E_FAIL;  // Not initialized
    }
    
    HRESULT hr = audioClient->Start();
    if (FAILED(hr)) {
        LOG_ERROR_F("Failed to start audio client: 0x%08X", hr);
        return hr;
    }
    
    try {
        isCapturing.store(true);
        captureThread = std::thread(&AudioCapture::CaptureThreadProc, this);
    } catch (const std::exception& e) {
        LOG_ERROR_F("Failed to start capture thread: %s", e.what());
        audioClient->Stop();
        return E_FAIL;
    }
    
    return S_OK;
}
```

#### Exception Pattern
```cpp
bool SpeechRecognition::Initialize(const SpeechConfig& config) {
    try {
        ValidateConfiguration(config);
        
        currentConfig_ = config;
        speechProvider_ = CreateProvider(config.provider);
        
        if (!speechProvider_->Initialize(config)) {
            throw SpeechRecognitionException(
                SpeechRecognitionException::ErrorType::InvalidConfiguration,
                "Failed to initialize speech provider");
        }
        
        initialized_ = true;
        return true;
        
    } catch (const SpeechRecognitionException& e) {
        LOG_ERROR_F("Speech recognition error: %s", e.what());
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR_F("Unexpected error initializing speech recognition: %s", e.what());
        return false;
    }
}

void SpeechRecognition::ValidateConfiguration(const SpeechConfig& config) {
    if (config.language.empty()) {
        throw SpeechRecognitionException(
            SpeechRecognitionException::ErrorType::InvalidConfiguration,
            "Language is required");
    }
    
    if (config.provider != Provider::Windows && config.apiKey.empty()) {
        throw SpeechRecognitionException(
            SpeechRecognitionException::ErrorType::InvalidConfiguration,
            "API key is required for cloud providers");
    }
}
```

#### Optional Pattern
```cpp
std::optional<AudioCapture::AudioFormat> AudioCapture::GetCurrentFormat() const {
    if (!isInitialized || !waveFormat) {
        return std::nullopt;
    }
    
    AudioFormat format;
    ConvertWaveFormatToAudioFormat(waveFormat, format);
    return format;
}

// Usage
auto format = audioCapture.GetCurrentFormat();
if (format) {
    std::cout << "Sample rate: " << format->sampleRate << std::endl;
} else {
    std::cout << "Audio format not available" << std::endl;
}
```

### 6.3 Error Recovery Strategies

#### Automatic Recovery
```cpp
class ErrorRecoveryManager {
public:
    static bool RecoverAudioCapture(AudioCapture* capture) {
        LOG_INFO("Attempting audio capture recovery");
        
        // Stop current capture
        capture->StopCapture();
        
        // Clean up resources
        capture->Cleanup();
        
        // Wait before retry
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Reinitialize
        HRESULT hr = capture->Initialize();
        if (SUCCEEDED(hr)) {
            LOG_INFO("Audio capture recovery successful");
            return true;
        } else {
            LOG_ERROR_F("Audio capture recovery failed: 0x%08X", hr);
            return false;
        }
    }
    
    static bool RecoverSpeechRecognition(SpeechRecognition* speech, 
                                        const SpeechRecognition::SpeechConfig& config) {
        LOG_INFO("Attempting speech recognition recovery");
        
        // Try with fallback provider (Windows Speech Recognition)
        auto fallbackConfig = config;
        fallbackConfig.provider = SpeechRecognition::Provider::Windows;
        fallbackConfig.apiKey = "";  // Not needed for Windows provider
        
        if (speech->Initialize(fallbackConfig)) {
            LOG_INFO("Speech recognition recovery successful (using Windows provider)");
            return true;
        } else {
            LOG_ERROR("Speech recognition recovery failed");
            return false;
        }
    }
};
```

---

## 7. Examples and Usage

### 7.1 Complete Application Example

```cpp
#include "MainWindow.h"
#include "AudioCapture.h"
#include "SpeechRecognition.h"
#include "ConfigManager.h"
#include "ExportManager.h"

class TranscriptionApplication {
public:
    bool Initialize() {
        // Initialize COM
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr)) {
            return false;
        }
        
        // Load configuration
        configManager_ = std::make_unique<ConfigManager>();
        if (!configManager_->LoadConfig()) {
            LOG_WARNING("Failed to load configuration, using defaults");
        }
        
        // Initialize audio capture
        audioCapture_ = std::make_unique<AudioCapture>();
        hr = audioCapture_->Initialize(AudioCapture::AudioQuality::Medium);
        if (FAILED(hr)) {
            LOG_ERROR_F("Failed to initialize audio capture: 0x%08X", hr);
            return false;
        }
        
        // Initialize speech recognition
        speechRecognition_ = std::make_unique<SpeechRecognition>();
        if (!speechRecognition_->Initialize(configManager_->GetSpeechConfig())) {
            LOG_ERROR("Failed to initialize speech recognition");
            return false;
        }
        
        // Set up callbacks
        SetupCallbacks();
        
        return true;
    }
    
    void StartRecording() {
        if (isRecording_) {
            return;
        }
        
        // Clear previous transcript
        currentTranscript_.segments.clear();
        currentTranscript_.startTime = std::chrono::system_clock::now();
        
        // Start audio capture
        HRESULT hr = audioCapture_->StartCapture();
        if (SUCCEEDED(hr)) {
            isRecording_ = true;
            LOG_INFO("Recording started");
        } else {
            LOG_ERROR_F("Failed to start recording: 0x%08X", hr);
        }
    }
    
    void StopRecording() {
        if (!isRecording_) {
            return;
        }
        
        audioCapture_->StopCapture();
        currentTranscript_.endTime = std::chrono::system_clock::now();
        isRecording_ = false;
        
        LOG_INFO("Recording stopped");
        
        // Auto-export if configured
        if (configManager_->GetConfig().autoExport) {
            ExportCurrentTranscript();
        }
    }
    
    void ExportCurrentTranscript() {
        ExportManager exportManager;
        
        std::string timestamp = FormatTimestamp(currentTranscript_.startTime);
        std::string baseFilename = "transcript_" + timestamp;
        
        // Export to configured formats
        for (const auto& format : configManager_->GetConfig().exportFormats) {
            ExportManager::Format exportFormat;
            if (format == "txt") exportFormat = ExportManager::Format::TXT;
            else if (format == "docx") exportFormat = ExportManager::Format::DOCX;
            else if (format == "pdf") exportFormat = ExportManager::Format::PDF;
            else continue;
            
            std::string filename = baseFilename + exportManager.GetFileExtension(exportFormat);
            
            if (exportManager.Export(currentTranscript_, exportFormat, filename)) {
                LOG_INFO_F("Exported transcript: %s", filename.c_str());
            } else {
                LOG_ERROR_F("Failed to export transcript: %s", filename.c_str());
            }
        }
    }

private:
    std::unique_ptr<AudioCapture> audioCapture_;
    std::unique_ptr<SpeechRecognition> speechRecognition_;
    std::unique_ptr<ConfigManager> configManager_;
    
    MeetingTranscript currentTranscript_;
    bool isRecording_ = false;
    
    void SetupCallbacks() {
        // Audio capture callback
        audioCapture_->SetAudioDataCallback([this](const std::vector<BYTE>& audioData,
                                                   const AudioCapture::AudioFormat& format) {
            if (isRecording_) {
                speechRecognition_->ProcessAudioData(audioData, format);
            }
        });
        
        // Speech recognition callback
        speechRecognition_->SetTranscriptionCallback([this](const std::string& text, double confidence) {
            if (!text.empty() && isRecording_) {
                TranscriptionSegment segment;
                segment.text = text;
                segment.confidence = confidence;
                segment.startTime = GetCurrentOffset();
                segment.endTime = segment.startTime + std::chrono::milliseconds(3000);  // Estimate
                segment.isFinal = true;
                
                currentTranscript_.segments.push_back(segment);
                
                // Notify UI or other components
                OnNewTranscriptionSegment(segment);
            }
        });
    }
    
    std::chrono::milliseconds GetCurrentOffset() {
        auto now = std::chrono::system_clock::now();
        auto duration = now - currentTranscript_.startTime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    }
    
    void OnNewTranscriptionSegment(const TranscriptionSegment& segment) {
        // Update UI, send notifications, etc.
        std::cout << "[" << segment.startTime.count() / 1000 << "s] " 
                  << segment.text << " (" << std::fixed << std::setprecision(1) 
                  << segment.confidence * 100 << "%)" << std::endl;
    }
    
    std::string FormatTimestamp(std::chrono::system_clock::time_point timePoint) {
        auto time_t = std::chrono::system_clock::to_time_t(timePoint);
        std::tm tm = *std::localtime(&time_t);
        
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
        return oss.str();
    }
};

// Application entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPWSTR lpCmdLine, int nCmdShow) {
    TranscriptionApplication app;
    
    if (!app.Initialize()) {
        MessageBox(nullptr, L"Failed to initialize application", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    
    // Create and run main window
    MainWindow window;
    if (window.Create(hInstance, nCmdShow)) {
        return window.MessageLoop();
    }
    
    return -1;
}
```

### 7.2 Plugin/Extension Example

```cpp
// Custom speech provider plugin
class PluginSpeechProvider : public SpeechRecognition::ISpeechProvider {
public:
    // Plugin interface
    static const char* GetPluginName() { return "Custom Provider Plugin"; }
    static const char* GetPluginVersion() { return "1.0.0"; }
    static const char* GetPluginDescription() { return "Custom speech recognition provider"; }
    
    // ISpeechProvider implementation
    bool Initialize(const SpeechRecognition::SpeechConfig& config) override {
        // Plugin-specific initialization
        return LoadPluginLibrary() && ConnectToService(config);
    }
    
    void ProcessAudioData(const std::vector<BYTE>& audioData,
                         const AudioCapture::AudioFormat& format) override {
        // Plugin-specific audio processing
        if (pluginInterface_) {
            pluginInterface_->ProcessAudio(audioData.data(), audioData.size(), format);
        }
    }
    
    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback callback) override {
        transcriptionCallback_ = callback;
        
        // Set up plugin callback
        if (pluginInterface_) {
            pluginInterface_->SetResultCallback([this](const char* text, double confidence) {
                if (transcriptionCallback_) {
                    transcriptionCallback_(std::string(text), confidence);
                }
            });
        }
    }
    
    bool IsInitialized() const override {
        return pluginInterface_ != nullptr;
    }

private:
    struct PluginInterface* pluginInterface_ = nullptr;
    SpeechRecognition::TranscriptionCallback transcriptionCallback_;
    
    bool LoadPluginLibrary() {
        // Load plugin DLL and get interface
        HMODULE hModule = LoadLibrary(L"CustomSpeechPlugin.dll");
        if (!hModule) return false;
        
        auto createInterface = (CreatePluginInterface*)GetProcAddress(hModule, "CreatePluginInterface");
        if (!createInterface) return false;
        
        pluginInterface_ = createInterface();
        return pluginInterface_ != nullptr;
    }
    
    bool ConnectToService(const SpeechRecognition::SpeechConfig& config) {
        if (!pluginInterface_) return false;
        
        PluginConfig pluginConfig;
        pluginConfig.apiKey = config.apiKey.c_str();
        pluginConfig.language = config.language.c_str();
        pluginConfig.enablePunctuation = config.enablePunctuation;
        
        return pluginInterface_->Initialize(&pluginConfig);
    }
};

// Plugin registration
void RegisterCustomProvider() {
    SpeechRecognition::RegisterProvider(
        SpeechRecognition::Provider::Custom,
        []() { return std::make_unique<PluginSpeechProvider>(); }
    );
}
```

---

## 8. Integration Guidelines

### 8.1 Threading Considerations

#### Thread Safety
The following components are thread-safe:
- `AudioCapture` (callback from audio thread)
- `Logger` (multiple threads may log simultaneously)
- `ConfigManager` (read operations only)

Not thread-safe (require external synchronization):
- `MainWindow` (UI thread only)
- `ExportManager` (single-threaded use)
- `SpeechRecognition` (internal synchronization, but not for external access)

#### Threading Best Practices
```cpp
// Audio processing on dedicated thread
class AudioProcessor {
private:
    std::thread audioThread_;
    std::atomic<bool> stopRequested_;
    std::queue<AudioData> audioQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    
public:
    void StartProcessing() {
        stopRequested_ = false;
        audioThread_ = std::thread([this]() { ProcessingLoop(); });
    }
    
    void StopProcessing() {
        stopRequested_ = true;
        queueCondition_.notify_all();
        if (audioThread_.joinable()) {
            audioThread_.join();
        }
    }
    
    void QueueAudioData(const AudioData& data) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        audioQueue_.push(data);
        queueCondition_.notify_one();
    }

private:
    void ProcessingLoop() {
        while (!stopRequested_) {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCondition_.wait(lock, [this]() { 
                return !audioQueue_.empty() || stopRequested_; 
            });
            
            while (!audioQueue_.empty() && !stopRequested_) {
                AudioData data = audioQueue_.front();
                audioQueue_.pop();
                lock.unlock();
                
                // Process audio data
                ProcessAudioChunk(data);
                
                lock.lock();
            }
        }
    }
};
```

### 8.2 Memory Management

#### RAII for Windows Resources
```cpp
// RAII wrapper for Windows resources
template<typename T>
class WindowsResource {
public:
    explicit WindowsResource(T* resource = nullptr) : resource_(resource) {}
    
    ~WindowsResource() {
        if (resource_) {
            resource_->Release();
        }
    }
    
    // Move semantics
    WindowsResource(WindowsResource&& other) noexcept : resource_(other.resource_) {
        other.resource_ = nullptr;
    }
    
    WindowsResource& operator=(WindowsResource&& other) noexcept {
        if (this != &other) {
            if (resource_) {
                resource_->Release();
            }
            resource_ = other.resource_;
            other.resource_ = nullptr;
        }
        return *this;
    }
    
    // Disable copy
    WindowsResource(const WindowsResource&) = delete;
    WindowsResource& operator=(const WindowsResource&) = delete;
    
    T* Get() const { return resource_; }
    T** GetAddressOf() { return &resource_; }
    T* Release() {
        T* temp = resource_;
        resource_ = nullptr;
        return temp;
    }

private:
    T* resource_;
};

// Usage
WindowsResource<IMMDeviceEnumerator> deviceEnumerator;
HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
                             IID_IMMDeviceEnumerator, 
                             reinterpret_cast<void**>(deviceEnumerator.GetAddressOf()));
```

### 8.3 Performance Optimization

#### Audio Buffer Management
```cpp
// Circular buffer for audio data
class CircularAudioBuffer {
public:
    CircularAudioBuffer(size_t capacity) : buffer_(capacity), capacity_(capacity) {}
    
    bool Write(const BYTE* data, size_t size) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (GetAvailableSpace() < size) {
            return false;  // Buffer full
        }
        
        for (size_t i = 0; i < size; ++i) {
            buffer_[writeIndex_] = data[i];
            writeIndex_ = (writeIndex_ + 1) % capacity_;
        }
        
        size_ += size;
        return true;
    }
    
    size_t Read(BYTE* data, size_t maxSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        size_t bytesToRead = std::min(maxSize, size_);
        
        for (size_t i = 0; i < bytesToRead; ++i) {
            data[i] = buffer_[readIndex_];
            readIndex_ = (readIndex_ + 1) % capacity_;
        }
        
        size_ -= bytesToRead;
        return bytesToRead;
    }
    
    size_t GetAvailableSpace() const {
        return capacity_ - size_;
    }
    
    size_t GetAvailableData() const {
        return size_;
    }

private:
    std::vector<BYTE> buffer_;
    size_t capacity_;
    size_t writeIndex_ = 0;
    size_t readIndex_ = 0;
    size_t size_ = 0;
    mutable std::mutex mutex_;
};
```

### 8.4 Error Handling Integration

#### Centralized Error Reporting
```cpp
class ErrorReporter {
public:
    enum class Severity {
        Info,
        Warning,
        Error,
        Critical
    };
    
    static void Report(Severity severity, const std::string& component, 
                      const std::string& message, HRESULT hr = S_OK) {
        ErrorReport report;
        report.timestamp = std::chrono::system_clock::now();
        report.severity = severity;
        report.component = component;
        report.message = message;
        report.errorCode = hr;
        report.threadId = GetCurrentThreadId();
        
        // Log the error
        Logger::GetInstance().Log(ConvertSeverity(severity), 
                                 FormatErrorMessage(report));
        
        // Store for analysis
        GetInstance().StoreReport(report);
        
        // Notify UI if critical
        if (severity == Severity::Critical) {
            GetInstance().NotifyUI(report);
        }
    }
    
    static std::vector<ErrorReport> GetRecentErrors(std::chrono::minutes timeWindow) {
        return GetInstance().GetErrorsInTimeWindow(timeWindow);
    }

private:
    struct ErrorReport {
        std::chrono::system_clock::time_point timestamp;
        Severity severity;
        std::string component;
        std::string message;
        HRESULT errorCode;
        DWORD threadId;
    };
    
    static ErrorReporter& GetInstance() {
        static ErrorReporter instance;
        return instance;
    }
    
    void StoreReport(const ErrorReport& report) {
        std::lock_guard<std::mutex> lock(reportsMutex_);
        recentReports_.push_back(report);
        
        // Keep only recent reports (last 1000 or last hour)
        auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(1);
        auto it = std::remove_if(recentReports_.begin(), recentReports_.end(),
                                [cutoff](const ErrorReport& r) { return r.timestamp < cutoff; });
        recentReports_.erase(it, recentReports_.end());
    }
    
    std::vector<ErrorReport> recentReports_;
    std::mutex reportsMutex_;
};

// Usage in components
class AudioCapture {
public:
    HRESULT StartCapture() {
        HRESULT hr = audioClient_->Start();
        if (FAILED(hr)) {
            ErrorReporter::Report(ErrorReporter::Severity::Error,
                                 "AudioCapture",
                                 "Failed to start audio client",
                                 hr);
            return hr;
        }
        return S_OK;
    }
};
```

---

## Document Information

**Document Version:** 2.0.0  
**Last Updated:** July 2025  
**Document Owner:** API Documentation Team  
**Review Cycle:** Quarterly  

### API Version Compatibility

| API Component | Version | Compatibility Notes |
|---------------|---------|-------------------|
| AudioCapture | 2.0.0 | Breaking changes from v1.x |
| SpeechRecognition | 2.0.0 | New provider interface |
| ConfigManager | 2.0.0 | Backward compatible with v1.x configs |
| ExportManager | 2.0.0 | New component in v2.0 |

### Related Documentation

- **Developer Guide**: [Developer_Documentation.md](Developer_Documentation.md)
- **User Manual**: [User_Manual.md](User_Manual.md)
- **Design Document**: [Detailed_Design_Document.md](Detailed_Design_Document.md)
- **Testing Guide**: [Testing_Documentation.md](Testing_Documentation.md)

### Support and Feedback

- **API Questions**: api-support@teamstranscription.com
- **Bug Reports**: Use GitHub Issues
- **Feature Requests**: Use GitHub Discussions
- **Documentation Feedback**: docs@teamstranscription.com

---

*© 2025 Teams Transcription Application. All rights reserved. This API documentation is provided for authorized developers and integrators only.*