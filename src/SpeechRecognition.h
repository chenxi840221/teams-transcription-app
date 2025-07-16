#pragma once

#include "AudioCapture.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

class SpeechRecognition {
public:
    enum class Provider {
        Azure,
        Google,
        OpenAI,
        AzureOpenAI,  // Azure OpenAI (GPT-4o)
        Amazon,
        Windows
    };

    struct SpeechConfig {
        Provider provider;
        std::string apiKey;
        std::string region;
        std::string language;
        std::string endpoint;  // Custom endpoint URL for Azure OpenAI
        std::string deployment; // Deployment name for Azure OpenAI
        bool enablePunctuation;
        bool enableSpeakerDiarization;
    };

    using TranscriptionCallback = std::function<void(const std::string& text, double confidence)>;
    
    // Forward declaration
    class ISpeechProvider;

    SpeechRecognition();
    ~SpeechRecognition();

    bool Initialize(const SpeechConfig& config);
    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format);
    void SetTranscriptionCallback(TranscriptionCallback callback);
    bool IsInitialized() const { return initialized; }

private:
    bool initialized;
    SpeechConfig currentConfig;
    TranscriptionCallback transcriptionCallback;
    std::unique_ptr<ISpeechProvider> speechProvider;

    bool InitializeProvider();
};
