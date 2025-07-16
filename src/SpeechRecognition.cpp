#include "SpeechRecognition.h"
#include "SimpleLogger.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <windows.h>
#include <winhttp.h>
#include <vector>
#include <string>

#pragma comment(lib, "winhttp.lib")

// Audio converter implementation
std::vector<BYTE> AudioConverter::ConvertAudioFormat(
    const std::vector<BYTE>& inputData,
    const AudioCapture::AudioFormat& inputFormat,
    AudioCapture::AudioFormat& outputFormat
) {
    // Set optimal output format for Azure OpenAI
    outputFormat.sampleRate = 16000;    // 16kHz is optimal for speech recognition
    outputFormat.channels = 1;          // Mono
    outputFormat.bitsPerSample = 16;    // 16-bit PCM
    outputFormat.bytesPerSecond = outputFormat.sampleRate * outputFormat.channels * (outputFormat.bitsPerSample / 8);
    
    // Convert 32-bit float input to 16-bit PCM
    size_t sampleCount = inputData.size() / sizeof(float);
    const float* floatData = reinterpret_cast<const float*>(inputData.data());
    std::vector<int16_t> pcmData = ConvertFloatToPCM16(floatData, sampleCount);
    
    // Convert stereo to mono if needed
    if (inputFormat.channels == 2) {
        pcmData = StereoToMono(pcmData);
    }
    
    // Downsample if needed
    if (inputFormat.sampleRate != outputFormat.sampleRate) {
        pcmData = Downsample(pcmData, inputFormat.sampleRate, outputFormat.sampleRate, inputFormat.channels == 2 ? 1 : inputFormat.channels);
    }
    
    // Convert back to BYTE vector
    std::vector<BYTE> result(pcmData.size() * sizeof(int16_t));
    memcpy(result.data(), pcmData.data(), result.size());
    
    return result;
}

std::vector<int16_t> AudioConverter::ConvertFloatToPCM16(const float* floatData, size_t sampleCount) {
    std::vector<int16_t> pcmData(sampleCount);
    
    for (size_t i = 0; i < sampleCount; ++i) {
        // Clamp float to [-1.0, 1.0] range and convert to 16-bit
        float sample = std::max(-1.0f, std::min(1.0f, floatData[i]));
        pcmData[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    return pcmData;
}

std::vector<int16_t> AudioConverter::Downsample(const std::vector<int16_t>& input, int inputRate, int outputRate, int channels) {
    if (inputRate == outputRate) {
        return input;
    }
    
    // Simple downsampling using decimation
    float ratio = static_cast<float>(inputRate) / outputRate;
    size_t outputSamples = static_cast<size_t>(input.size() / ratio);
    std::vector<int16_t> output(outputSamples);
    
    for (size_t i = 0; i < outputSamples; ++i) {
        size_t inputIndex = static_cast<size_t>(i * ratio);
        if (inputIndex < input.size()) {
            output[i] = input[inputIndex];
        }
    }
    
    return output;
}

std::vector<int16_t> AudioConverter::StereoToMono(const std::vector<int16_t>& stereoData) {
    std::vector<int16_t> monoData(stereoData.size() / 2);
    
    for (size_t i = 0; i < monoData.size(); ++i) {
        // Average left and right channels
        int32_t left = stereoData[i * 2];
        int32_t right = stereoData[i * 2 + 1];
        monoData[i] = static_cast<int16_t>((left + right) / 2);
    }
    
    return monoData;
}

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

        // Process every 1 second of audio for lower latency (reduced from 3 seconds)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTranscription).count();
        
        size_t bytesPerSecond = format.sampleRate * format.channels * (format.bitsPerSample / 8);
        size_t oneSecondOfAudio = bytesPerSecond * 1;  // Reduced buffer size for real-time
        
        DEBUG_LOG("AzureOpenAI - Buffer: " + std::to_string(audioBuffer.size()) + "/" + std::to_string(oneSecondOfAudio) + " bytes, Elapsed: " + std::to_string(elapsed) + "s");
        
        if (audioBuffer.size() >= oneSecondOfAudio && elapsed >= 1) {
            INFO_LOG("AzureOpenAI processing audio chunk - " + std::to_string(audioBuffer.size()) + " bytes");
            
            // Convert audio format for optimal Azure OpenAI processing
            AudioCapture::AudioFormat optimizedFormat;
            std::vector<BYTE> convertedAudio = AudioConverter::ConvertAudioFormat(audioBuffer, format, optimizedFormat);
            INFO_LOG("Audio converted: " + std::to_string(audioBuffer.size()) + " -> " + std::to_string(convertedAudio.size()) + " bytes, " +
                     std::to_string(format.sampleRate) + "Hz -> " + std::to_string(optimizedFormat.sampleRate) + "Hz, " +
                     std::to_string(format.channels) + "ch -> " + std::to_string(optimizedFormat.channels) + "ch");
            
            // Create WAV file with optimized format
            std::vector<BYTE> wavData = CreateWavFile(convertedAudio, optimizedFormat);
            INFO_LOG("Created optimized WAV file: " + std::to_string(wavData.size()) + " bytes");
            
            // Log WAV sample for debugging
            static int wavCount = 0;
            DEBUG_LOG("Created WAV sample #" + std::to_string(++wavCount) + " - " + std::to_string(wavData.size()) + " bytes");
            
            // Send to Azure OpenAI for transcription (async in future implementation)
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
        // Check if we have sufficient audio data (at least 0.5 seconds of audio for real-time)
        // With optimized format: 16kHz * 1 channel * 2 bytes per sample * 0.5 seconds = 16,000 bytes
        if (wavData.size() < 16000) {
            DEBUG_LOG("AzureOpenAI - Insufficient audio data: " + std::to_string(wavData.size()) + " bytes (minimum 16,000 bytes)");
            return ""; // Not enough audio data yet, no transcription
        }
        
        INFO_LOG("AzureOpenAI - Processing " + std::to_string(wavData.size()) + " bytes of audio for transcription");
        
        try {
            return SendAudioToAzureOpenAI(wavData);
        }
        catch (const std::exception& e) {
            ERROR_LOG("AzureOpenAI HTTP request failed: " + std::string(e.what()));
            return "";
        }
    }
    
private:
    std::string SendAudioToAzureOpenAI(const std::vector<BYTE>& wavData) {
        // Parse the endpoint URL
        std::wstring url = std::wstring(config.endpoint.begin(), config.endpoint.end());
        
        // Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(L"TeamsTranscriptionApp/1.0",
                                       WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                       WINHTTP_NO_PROXY_NAME,
                                       WINHTTP_NO_PROXY_BYPASS,
                                       0);
        if (!hSession) {
            throw std::runtime_error("Failed to initialize WinHTTP session");
        }
        
        // Parse URL components
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        wchar_t hostName[256];
        wchar_t urlPath[1024];
        urlComp.lpszHostName = hostName;
        urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
        urlComp.lpszUrlPath = urlPath;
        urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);
        
        if (!WinHttpCrackUrl(url.c_str(), 0, 0, &urlComp)) {
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to parse Azure OpenAI URL");
        }
        
        // Connect to server
        HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName, urlComp.nPort, 0);
        if (!hConnect) {
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to connect to Azure OpenAI server");
        }
        
        // Create request
        DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", urlComp.lpszUrlPath,
                                              NULL, WINHTTP_NO_REFERER,
                                              WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to create HTTP request");
        }
        
        // Prepare multipart form data
        std::string boundary = "----WebKitFormBoundary" + std::to_string(GetTickCount64());
        std::string contentType = "multipart/form-data; boundary=" + boundary;
        std::wstring contentTypeW = std::wstring(contentType.begin(), contentType.end());
        
        // Set headers
        std::wstring apiKey = std::wstring(config.apiKey.begin(), config.apiKey.end());
        std::wstring authHeader = L"api-key: " + apiKey;
        
        if (!WinHttpAddRequestHeaders(hRequest, authHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD) ||
            !WinHttpAddRequestHeaders(hRequest, contentTypeW.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to set HTTP headers");
        }
        
        // Build multipart body
        std::vector<BYTE> requestBody = BuildMultipartBody(wavData, boundary);
        
        // Send request
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                              requestBody.data(), requestBody.size(),
                              requestBody.size(), 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to send HTTP request");
        }
        
        // Receive response
        if (!WinHttpReceiveResponse(hRequest, NULL)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to receive HTTP response");
        }
        
        // Get status code
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                          NULL, &statusCode, &statusCodeSize, NULL);
        
        // Read response body
        std::string responseData;
        DWORD bytesAvailable = 0;
        while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
            std::vector<char> buffer(bytesAvailable + 1);
            DWORD bytesRead = 0;
            if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
                buffer[bytesRead] = '\0';
                responseData.append(buffer.data(), bytesRead);
            }
        }
        
        // Cleanup
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        
        // Parse response
        if (statusCode != 200) {
            ERROR_LOG("Azure OpenAI API returned status code: " + std::to_string(statusCode) + ", response: " + responseData);
            return "";
        }
        
        INFO_LOG("Azure OpenAI response: " + responseData);
        return ParseTranscriptionResponse(responseData);
    }
    
    std::vector<BYTE> BuildMultipartBody(const std::vector<BYTE>& wavData, const std::string& boundary) {
        std::vector<BYTE> body;
        
        // Add file part
        std::string fileHeader = "--" + boundary + "\r\n";
        fileHeader += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n";
        fileHeader += "Content-Type: audio/wav\r\n\r\n";
        
        body.insert(body.end(), fileHeader.begin(), fileHeader.end());
        body.insert(body.end(), wavData.begin(), wavData.end());
        
        // Add model parameter
        std::string modelParam = "\r\n--" + boundary + "\r\n";
        modelParam += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
        modelParam += "whisper-1\r\n";
        
        body.insert(body.end(), modelParam.begin(), modelParam.end());
        
        // Add language parameter
        std::string langParam = "--" + boundary + "\r\n";
        langParam += "Content-Disposition: form-data; name=\"language\"\r\n\r\n";
        langParam += "en\r\n";
        
        body.insert(body.end(), langParam.begin(), langParam.end());
        
        // Add response format parameter
        std::string formatParam = "--" + boundary + "\r\n";
        formatParam += "Content-Disposition: form-data; name=\"response_format\"\r\n\r\n";
        formatParam += "json\r\n";
        
        body.insert(body.end(), formatParam.begin(), formatParam.end());
        
        // Close boundary
        std::string endBoundary = "--" + boundary + "--\r\n";
        body.insert(body.end(), endBoundary.begin(), endBoundary.end());
        
        return body;
    }
    
    std::string ParseTranscriptionResponse(const std::string& jsonResponse) {
        try {
            // Simple JSON parsing to extract "text" field
            size_t textPos = jsonResponse.find("\"text\"");
            if (textPos == std::string::npos) {
                WARN_LOG("No 'text' field found in Azure OpenAI response");
                return "";
            }
            
            size_t colonPos = jsonResponse.find(":", textPos);
            size_t quoteStart = jsonResponse.find("\"", colonPos);
            size_t quoteEnd = jsonResponse.find("\"", quoteStart + 1);
            
            if (quoteStart == std::string::npos || quoteEnd == std::string::npos) {
                WARN_LOG("Failed to parse transcription text from response");
                return "";
            }
            
            std::string transcription = jsonResponse.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            
            // Clean up the transcription (remove extra whitespace, etc.)
            if (!transcription.empty() && transcription != " ") {
                INFO_LOG("Successfully parsed transcription: '" + transcription + "'");
                return transcription;
            }
            
            return "";
        }
        catch (const std::exception& e) {
            ERROR_LOG("Error parsing Azure OpenAI response: " + std::string(e.what()));
            return "";
        }
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