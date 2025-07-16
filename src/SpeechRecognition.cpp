#include "SpeechRecognition.h"
#include "SimpleLogger.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

// Abstract interface for speech providers
class SpeechRecognition::ISpeechProvider {
public:
    virtual ~ISpeechProvider() = default;
    virtual bool Initialize(const SpeechConfig& config) = 0;
    virtual void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) = 0;
    virtual void SetTranscriptionCallback(TranscriptionCallback callback) = 0;
    virtual bool IsInitialized() const = 0;
};

// Azure Speech Services Provider
class AzureSpeechProvider : public SpeechRecognition::ISpeechProvider {
private:
    bool initialized;
    SpeechRecognition::SpeechConfig config;
    SpeechRecognition::TranscriptionCallback callback;
    std::vector<BYTE> audioBuffer;
    size_t bufferThreshold;

public:
    AzureSpeechProvider() : initialized(false), bufferThreshold(32000) {} // ~1 second at 16kHz

    bool Initialize(const SpeechRecognition::SpeechConfig& speechConfig) override {
        config = speechConfig;
        
        if (config.apiKey.empty()) {
            std::cerr << "Azure Speech API key is required" << std::endl;
            return false;
        }

        // In a real implementation, this would initialize the Azure Speech SDK
        // For now, this is a stub that simulates the initialization
        initialized = true;
        std::cout << "Azure Speech Provider initialized (stub)" << std::endl;
        return true;
    }

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) override {
        if (!initialized || !callback) {
            WARN_LOG("AzureSpeechProvider::ProcessAudioData - Not initialized (" + std::string(initialized ? "true" : "false") + ") or no callback (" + std::string(callback ? "set" : "null") + ")");
            return;
        }

        // Accumulate audio data
        audioBuffer.insert(audioBuffer.end(), audioData.begin(), audioData.end());
        AUDIO_LOG("AzureSpeechProvider", audioData.size(), "Buffer total: " + std::to_string(audioBuffer.size()) + "/" + std::to_string(bufferThreshold));

        // Process when we have enough data
        if (audioBuffer.size() >= bufferThreshold) {
            INFO_LOG("AzureSpeechProvider processing accumulated audio buffer");
            ProcessAccumulatedAudio(format);
            audioBuffer.clear();
        }
    }

    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback transcriptionCallback) override {
        callback = transcriptionCallback;
        INFO_LOG("AzureSpeechProvider transcription callback set");
    }

    bool IsInitialized() const override {
        return initialized;
    }

private:
    void ProcessAccumulatedAudio(const AudioCapture::AudioFormat& format) {
        // This is a stub implementation for demonstration
        // In a real implementation, this would send audio to Azure Speech Services
        
        // Simulate processing delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Generate a simulated transcription based on audio characteristics
        std::string transcription = GenerateSimulatedTranscription();
        double confidence = 0.85; // Simulated confidence score

        if (callback && !transcription.empty()) {
            INFO_LOG("AzureSpeechProvider calling transcription callback with: '" + transcription + "'");
            callback(transcription, confidence);
        } else {
            WARN_LOG("AzureSpeechProvider - No callback (" + std::string(callback ? "set" : "null") + ") or empty transcription");
        }
    }

    std::string GenerateSimulatedTranscription() {
        // This generates simulated transcription text for demonstration
        // In a real implementation, this would be the actual transcription from Azure
        
        static int counter = 0;
        counter++;

        std::vector<std::string> samplePhrases = {
            "Thank you for joining today's meeting.",
            "Let's start with the agenda items.",
            "Can everyone see the screen share?",
            "I'll send the follow-up notes after this call.",
            "Does anyone have questions about this topic?",
            "Let's move on to the next item.",
            "I think we should schedule a follow-up meeting.",
            "The quarterly results look promising.",
            "We need to finalize the project timeline.",
            "Can you please mute your microphone?"
        };

        if (counter % 10 == 0) { // Return a phrase every 10th call
            return samplePhrases[counter / 10 % samplePhrases.size()];
        }

        return ""; // Return empty most of the time to simulate realistic speech gaps
    }
};

// Google Speech Provider (stub)
class GoogleSpeechProvider : public SpeechRecognition::ISpeechProvider {
private:
    bool initialized;

public:
    GoogleSpeechProvider() : initialized(false) {}

    bool Initialize(const SpeechRecognition::SpeechConfig& config) override {
        // Stub implementation
        initialized = !config.apiKey.empty();
        if (initialized) {
            std::cout << "Google Speech Provider initialized (stub)" << std::endl;
        }
        return initialized;
    }

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) override {
        // Stub implementation
    }

    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback callback) override {
        // Stub implementation
    }

    bool IsInitialized() const override {
        return initialized;
    }
};

// OpenAI Whisper Provider (stub)
class OpenAISpeechProvider : public SpeechRecognition::ISpeechProvider {
private:
    bool initialized;

public:
    OpenAISpeechProvider() : initialized(false) {}

    bool Initialize(const SpeechRecognition::SpeechConfig& config) override {
        // Stub implementation
        initialized = !config.apiKey.empty();
        if (initialized) {
            std::cout << "OpenAI Speech Provider initialized (stub)" << std::endl;
        }
        return initialized;
    }

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) override {
        // Stub implementation
    }

    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback callback) override {
        // Stub implementation
    }

    bool IsInitialized() const override {
        return initialized;
    }
};

// Azure OpenAI Speech Provider (GPT-4o)
class AzureOpenAISpeechProvider : public SpeechRecognition::ISpeechProvider {
private:
    bool initialized;
    SpeechRecognition::SpeechConfig config;
    SpeechRecognition::TranscriptionCallback callback;
    std::vector<BYTE> audioBuffer;
    std::chrono::steady_clock::time_point lastTranscription;

public:
    AzureOpenAISpeechProvider() : initialized(false) {}

    bool Initialize(const SpeechRecognition::SpeechConfig& speechConfig) override {
        config = speechConfig;
        
        if (config.apiKey.empty() || config.endpoint.empty()) {
            std::cerr << "Azure OpenAI API key and endpoint are required" << std::endl;
            return false;
        }
        
        initialized = true;
        lastTranscription = std::chrono::steady_clock::now();
        std::cout << "Azure OpenAI Speech Provider (GPT-4o) initialized" << std::endl;
        std::cout << "Endpoint: " << config.endpoint << std::endl;
        std::cout << "Deployment: " << config.deployment << std::endl;
        return true;
    }

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) override {
        if (!initialized || !callback) {
            WARN_LOG("AzureOpenAISpeechProvider::ProcessAudioData - Not initialized (" + std::string(initialized ? "true" : "false") + ") or no callback (" + std::string(callback ? "set" : "null") + ")");
            return;
        }

        // Accumulate audio data
        audioBuffer.insert(audioBuffer.end(), audioData.begin(), audioData.end());
        AUDIO_LOG("AzureOpenAISpeechProvider", audioData.size(), "Buffer total: " + std::to_string(audioBuffer.size()));

        // Process every 3 seconds of audio for better transcription
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTranscription).count();
        
        size_t bytesPerSecond = format.sampleRate * format.channels * (format.bitsPerSample / 8);
        size_t threeSecondsOfAudio = bytesPerSecond * 3;
        
        DEBUG_LOG("AzureOpenAI - Buffer: " + std::to_string(audioBuffer.size()) + "/" + std::to_string(threeSecondsOfAudio) + " bytes, Elapsed: " + std::to_string(elapsed) + "s");
        
        if (audioBuffer.size() >= threeSecondsOfAudio && elapsed >= 3) {
            INFO_LOG("AzureOpenAI processing audio chunk - " + std::to_string(audioBuffer.size()) + " bytes");
            
            // Convert audio to WAV format for Azure OpenAI
            std::vector<BYTE> wavData = CreateWavFile(audioBuffer, format);
            INFO_LOG("Created WAV file: " + std::to_string(wavData.size()) + " bytes");
            
            // Log WAV sample for debugging
            static int wavCount = 0;
            DEBUG_LOG("Created WAV sample #" + std::to_string(++wavCount) + " - " + std::to_string(wavData.size()) + " bytes");
            
            // Send to Azure OpenAI for transcription
            std::string transcriptionText = SendToAzureOpenAI(wavData);
            
            if (!transcriptionText.empty() && callback) {
                INFO_LOG("AzureOpenAI transcription successful: '" + transcriptionText + "'");
                callback(transcriptionText, 0.95);
                std::cout << "Azure OpenAI transcription: " << transcriptionText << std::endl;
            } else {
                WARN_LOG("AzureOpenAI - Empty transcription or no callback");
            }
            
            // Clear buffer and update timestamp
            audioBuffer.clear();
            lastTranscription = now;
        }
    }

    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback cb) override {
        callback = cb;
        INFO_LOG("AzureOpenAI transcription callback set");
        std::cout << "Azure OpenAI transcription callback set" << std::endl;
    }

    bool IsInitialized() const override {
        return initialized;
    }

private:
    std::vector<BYTE> CreateWavFile(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) {
        std::vector<BYTE> wavFile;
        
        // WAV header (44 bytes)
        uint32_t dataSize = audioData.size();
        uint32_t fileSize = dataSize + 36;
        uint16_t channels = format.channels;
        uint32_t sampleRate = format.sampleRate;
        uint16_t bitsPerSample = format.bitsPerSample;
        uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
        uint16_t blockAlign = channels * (bitsPerSample / 8);
        
        // RIFF header
        wavFile.insert(wavFile.end(), {'R', 'I', 'F', 'F'});
        wavFile.insert(wavFile.end(), (BYTE*)&fileSize, (BYTE*)&fileSize + 4);
        wavFile.insert(wavFile.end(), {'W', 'A', 'V', 'E'});
        
        // fmt chunk
        wavFile.insert(wavFile.end(), {'f', 'm', 't', ' '});
        uint32_t fmtSize = 16;
        wavFile.insert(wavFile.end(), (BYTE*)&fmtSize, (BYTE*)&fmtSize + 4);
        uint16_t audioFormat = 1; // PCM
        wavFile.insert(wavFile.end(), (BYTE*)&audioFormat, (BYTE*)&audioFormat + 2);
        wavFile.insert(wavFile.end(), (BYTE*)&channels, (BYTE*)&channels + 2);
        wavFile.insert(wavFile.end(), (BYTE*)&sampleRate, (BYTE*)&sampleRate + 4);
        wavFile.insert(wavFile.end(), (BYTE*)&byteRate, (BYTE*)&byteRate + 4);
        wavFile.insert(wavFile.end(), (BYTE*)&blockAlign, (BYTE*)&blockAlign + 2);
        wavFile.insert(wavFile.end(), (BYTE*)&bitsPerSample, (BYTE*)&bitsPerSample + 2);
        
        // data chunk
        wavFile.insert(wavFile.end(), {'d', 'a', 't', 'a'});
        wavFile.insert(wavFile.end(), (BYTE*)&dataSize, (BYTE*)&dataSize + 4);
        wavFile.insert(wavFile.end(), audioData.begin(), audioData.end());
        
        return wavFile;
    }
    
    std::string SendToAzureOpenAI(const std::vector<BYTE>& wavData) {
        // Check if we have sufficient audio data (at least 3 seconds of audio)
        if (wavData.size() < 44100 * 2 * 2 * 3) { // 44.1kHz * 2 channels * 2 bytes per sample * 3 seconds
            DEBUG_LOG("AzureOpenAI - Insufficient audio data: " + std::to_string(wavData.size()) + " bytes");
            return ""; // Not enough audio data yet, no transcription
        }
        
        INFO_LOG("AzureOpenAI - Processing " + std::to_string(wavData.size()) + " bytes of audio for transcription");
        
        // TODO: Real HTTP implementation to Azure OpenAI would go here
        // For now, return empty to prevent mocked transcription appearing without real audio
        INFO_LOG("AzureOpenAI - No transcription generated (awaiting real implementation)");
        return "";
    }
};

// Windows Speech Recognition Provider (stub)
class WindowsSpeechProvider : public SpeechRecognition::ISpeechProvider {
private:
    bool initialized;
    SpeechRecognition::TranscriptionCallback callback;
    std::vector<BYTE> audioBuffer;
    static int transcriptionCounter;

public:
    WindowsSpeechProvider() : initialized(false) {}

    bool Initialize(const SpeechRecognition::SpeechConfig& config) override {
        initialized = true;
        std::cout << "Windows Speech Provider initialized" << std::endl;
        return true;
    }

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) override {
        if (!initialized || !callback) {
            WARN_LOG("WindowsSpeechProvider::ProcessAudioData - Not initialized (" + std::string(initialized ? "true" : "false") + ") or no callback (" + std::string(callback ? "set" : "null") + ")");
            return;
        }

        // Accumulate audio data
        audioBuffer.insert(audioBuffer.end(), audioData.begin(), audioData.end());
        AUDIO_LOG("WindowsSpeechProvider", audioData.size(), "Buffer total: " + std::to_string(audioBuffer.size()));

        // Process every ~1 second of audio (simple demo implementation)
        size_t bytesPerSecond = format.sampleRate * format.channels * (format.bitsPerSample / 8);
        DEBUG_LOG("Windows Speech - Buffer: " + std::to_string(audioBuffer.size()) + "/" + std::to_string(bytesPerSecond) + " bytes");
        
        if (audioBuffer.size() >= bytesPerSecond) {
            INFO_LOG("Windows Speech processing audio chunk - " + std::to_string(audioBuffer.size()) + " bytes");
            
            // Simple test: Generate demo transcription to verify the pipeline works
            std::string demoText = "Demo transcription #" + std::to_string(++transcriptionCounter) + 
                                 " - Audio detected (" + std::to_string(audioBuffer.size()) + " bytes)";
            
            // Call the callback with demo text
            if (callback) {
                INFO_LOG("Windows Speech calling callback with: '" + demoText + "'");
                callback(demoText, 0.95); // High confidence demo
            }
            
            // Clear buffer for next chunk
            audioBuffer.clear();
            
            std::cout << "Processed audio chunk: " << demoText << std::endl;
        }
    }

    void SetTranscriptionCallback(SpeechRecognition::TranscriptionCallback cb) override {
        callback = cb;
        INFO_LOG("Windows Speech transcription callback set");
        std::cout << "Transcription callback set" << std::endl;
    }

    bool IsInitialized() const override {
        return initialized;
    }
};

// Static member definition
int WindowsSpeechProvider::transcriptionCounter = 0;

// SpeechRecognition Implementation
SpeechRecognition::SpeechRecognition()
    : initialized(false)
{
}

SpeechRecognition::~SpeechRecognition() = default;

bool SpeechRecognition::Initialize(const SpeechConfig& config) {
    currentConfig = config;
    INFO_LOG("SpeechRecognition::Initialize - Provider: " + std::to_string((int)config.provider) + 
             ", Endpoint: " + config.endpoint + ", API Key: " + (config.apiKey.empty() ? "EMPTY" : "SET"));
    return InitializeProvider();
}

bool SpeechRecognition::InitializeProvider() {
    speechProvider.reset();

    try {
        std::string providerName;
        switch (currentConfig.provider) {
            case Provider::Azure:
                speechProvider = std::make_unique<AzureSpeechProvider>();
                providerName = "Azure";
                break;

            case Provider::Google:
                speechProvider = std::make_unique<GoogleSpeechProvider>();
                providerName = "Google";
                break;

            case Provider::OpenAI:
                speechProvider = std::make_unique<OpenAISpeechProvider>();
                providerName = "OpenAI";
                break;

            case Provider::AzureOpenAI:
                speechProvider = std::make_unique<AzureOpenAISpeechProvider>();
                providerName = "AzureOpenAI";
                break;

            case Provider::Windows:
                speechProvider = std::make_unique<WindowsSpeechProvider>();
                providerName = "Windows";
                break;

            default:
                ERROR_LOG("Unknown speech provider: " + std::to_string((int)currentConfig.provider));
                std::cerr << "Unknown speech provider" << std::endl;
                return false;
        }

        INFO_LOG("Created " + providerName + " speech provider, initializing...");

        if (!speechProvider->Initialize(currentConfig)) {
            ERROR_LOG("Failed to initialize " + providerName + " speech provider");
            std::cerr << "Failed to initialize speech provider" << std::endl;
            speechProvider.reset();
            return false;
        }

        // Set up the transcription callback
        if (transcriptionCallback) {
            INFO_LOG("Setting transcription callback on " + providerName + " provider");
            speechProvider->SetTranscriptionCallback(transcriptionCallback);
        } else {
            WARN_LOG("No transcription callback available when initializing " + providerName + " provider");
        }

        initialized = speechProvider->IsInitialized();
        INFO_LOG(providerName + " speech provider initialization " + (initialized ? "SUCCESS" : "FAILED"));
        return initialized;
    }
    catch (const std::exception& e) {
        ERROR_LOG("Exception initializing speech provider: " + std::string(e.what()));
        std::cerr << "Exception initializing speech provider: " << e.what() << std::endl;
        speechProvider.reset();
        return false;
    }
}

void SpeechRecognition::ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) {
    if (!initialized || !speechProvider) {
        static int warnCount = 0;
        if (++warnCount <= 5) { // Only warn first 5 times to avoid spam
            WARN_LOG("SpeechRecognition::ProcessAudioData - Not initialized (" + std::string(initialized ? "true" : "false") + ") or no provider (" + std::string(speechProvider ? "set" : "null") + ")");
        }
        return;
    }

    try {
        DEBUG_LOG("SpeechRecognition forwarding " + std::to_string(audioData.size()) + " bytes to provider");
        speechProvider->ProcessAudioData(audioData, format);
    }
    catch (const std::exception& e) {
        ERROR_LOG("Exception processing audio data: " + std::string(e.what()));
        std::cerr << "Exception processing audio data: " << e.what() << std::endl;
    }
}

void SpeechRecognition::SetTranscriptionCallback(TranscriptionCallback callback) {
    transcriptionCallback = callback;
    INFO_LOG("SpeechRecognition::SetTranscriptionCallback called");
    
    if (speechProvider) {
        INFO_LOG("Setting callback on existing speech provider");
        speechProvider->SetTranscriptionCallback(callback);
    } else {
        WARN_LOG("No speech provider available when setting transcription callback");
    }
}