#pragma once

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

class AudioCapture {
public:
    enum class AudioQuality {
        Low = 8000,
        Medium = 16000,
        High = 44100
    };

    struct AudioFormat {
        UINT32 sampleRate;
        UINT16 channels;
        UINT16 bitsPerSample;
        UINT32 bytesPerSecond;
    };

    using AudioDataCallback = std::function<void(const std::vector<BYTE>&, const AudioFormat&)>;

    AudioCapture();
    ~AudioCapture();

    HRESULT Initialize(AudioQuality quality = AudioQuality::Medium);
    HRESULT StartCapture();
    void StopCapture();
    bool IsCapturing() const { return isCapturing.load(); }
    void SetAudioDataCallback(AudioDataCallback callback);
    AudioFormat GetAudioFormat() const;

    struct CaptureStats {
        UINT64 totalFramesCaptured;
        UINT64 totalBytesProcessed;
        double captureTimeSeconds;
        UINT32 bufferUnderruns;
    };
    CaptureStats GetCaptureStats() const;

private:
    IMMDeviceEnumerator* deviceEnumerator;
    IMMDevice* audioDevice;
    IAudioClient* audioClient;
    IAudioCaptureClient* captureClient;
    WAVEFORMATEX* waveFormat;

    std::atomic<bool> isCapturing;
    std::thread captureThread;
    mutable std::mutex statsMutex;

    AudioDataCallback audioCallback;
    std::vector<BYTE> audioBuffer;
    AudioFormat currentFormat;
    mutable CaptureStats stats;

    void CaptureThreadProc();
    void ProcessAudioData(BYTE* audioData, UINT32 numFrames, DWORD flags);
    void UpdateStats(UINT32 framesProcessed, UINT32 bytesProcessed);
    void Cleanup();
    HRESULT InitializeAudioClient(AudioQuality quality);
    void ConvertWaveFormatToAudioFormat(const WAVEFORMATEX* wfx, AudioFormat& format);
};
