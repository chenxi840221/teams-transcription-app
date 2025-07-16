#pragma once

#include "AudioCapture.h"
#include <string>
#include <functional>
#include <memory>
#include <vector>

// Audio conversion utilities
class AudioConverter {
public:
    // Convert audio format for optimal Azure OpenAI processing
    static std::vector<BYTE> ConvertAudioFormat(
        const std::vector<BYTE>& inputData,
        const AudioCapture::AudioFormat& inputFormat,
        AudioCapture::AudioFormat& outputFormat
    );
    
private:
    // Convert 32-bit float to 16-bit PCM
    static std::vector<int16_t> ConvertFloatToPCM16(const float* floatData, size_t sampleCount);
    
    // Downsample audio data
    static std::vector<int16_t> Downsample(const std::vector<int16_t>& input, int inputRate, int outputRate, int channels);
    
    // Convert stereo to mono
    static std::vector<int16_t> StereoToMono(const std::vector<int16_t>& stereoData);
};

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
