#include "AudioCapture.h"
#include <iostream>
#include <chrono>

#pragma comment(lib, "mmdevapi.lib")
#pragma comment(lib, "ole32.lib")

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

AudioCapture::AudioCapture()
    : deviceEnumerator(nullptr)
    , audioDevice(nullptr)
    , audioClient(nullptr)
    , captureClient(nullptr)
    , waveFormat(nullptr)
    , isCapturing(false)
{
    memset(&stats, 0, sizeof(stats));
}

AudioCapture::~AudioCapture() {
    StopCapture();
    Cleanup();
}

HRESULT AudioCapture::Initialize(AudioQuality quality) {
    HRESULT hr = S_OK;

    // Initialize COM
    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL,
        IID_IMMDeviceEnumerator, (void**)&deviceEnumerator);
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    // Get default audio endpoint (speakers/headphones for loopback)
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &audioDevice);
    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio endpoint: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    // Activate audio client
    hr = audioDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&audioClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to activate audio client: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    // Initialize audio client for loopback capture
    hr = InitializeAudioClient(quality);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    // Get the capture client
    hr = audioClient->GetService(IID_IAudioCaptureClient, (void**)&captureClient);
    if (FAILED(hr)) {
        std::cerr << "Failed to get capture client: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    std::cout << "Audio capture initialized successfully" << std::endl;
    return S_OK;
}

HRESULT AudioCapture::InitializeAudioClient(AudioQuality quality) {
    HRESULT hr = S_OK;

    // Get the mix format
    hr = audioClient->GetMixFormat(&waveFormat);
    if (FAILED(hr)) {
        return hr;
    }

    // Convert wave format to our format structure
    ConvertWaveFormatToAudioFormat(waveFormat, currentFormat);

    // Adjust format based on quality setting
    WAVEFORMATEX adjustedFormat = *waveFormat;
    adjustedFormat.nSamplesPerSec = static_cast<DWORD>(quality);
    adjustedFormat.nAvgBytesPerSec = adjustedFormat.nSamplesPerSec * adjustedFormat.nChannels * adjustedFormat.wBitsPerSample / 8;

    // Initialize the audio client for loopback capture
    hr = audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,  // Loopback capture
        10000000,  // 1 second buffer
        0,
        &adjustedFormat,
        nullptr);

    if (FAILED(hr)) {
        std::cerr << "Failed to initialize audio client with adjusted format, trying original format" << std::endl;
        // Try with original format if adjusted format fails
        hr = audioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            10000000,
            0,
            waveFormat,
            nullptr);
    }

    return hr;
}

void AudioCapture::ConvertWaveFormatToAudioFormat(const WAVEFORMATEX* wfx, AudioFormat& format) {
    if (wfx) {
        format.sampleRate = wfx->nSamplesPerSec;
        format.channels = wfx->nChannels;
        format.bitsPerSample = wfx->wBitsPerSample;
        format.bytesPerSecond = wfx->nAvgBytesPerSec;
    }
}

HRESULT AudioCapture::StartCapture() {
    if (isCapturing.load()) {
        return S_OK;  // Already capturing
    }

    if (!audioClient || !captureClient) {
        return E_FAIL;
    }

    HRESULT hr = audioClient->Start();
    if (FAILED(hr)) {
        std::cerr << "Failed to start audio client: 0x" << std::hex << hr << std::endl;
        return hr;
    }

    isCapturing.store(true);
    captureThread = std::thread(&AudioCapture::CaptureThreadProc, this);

    std::cout << "Audio capture started" << std::endl;
    return S_OK;
}

void AudioCapture::StopCapture() {
    if (!isCapturing.load()) {
        return;  // Not capturing
    }

    isCapturing.store(false);

    if (captureThread.joinable()) {
        captureThread.join();
    }

    if (audioClient) {
        audioClient->Stop();
    }

    std::cout << "Audio capture stopped" << std::endl;
}

void AudioCapture::SetAudioDataCallback(AudioDataCallback callback) {
    audioCallback = callback;
}

AudioCapture::AudioFormat AudioCapture::GetAudioFormat() const {
    return currentFormat;
}

AudioCapture::CaptureStats AudioCapture::GetCaptureStats() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return stats;
}

void AudioCapture::CaptureThreadProc() {
    auto startTime = std::chrono::high_resolution_clock::now();

    while (isCapturing.load()) {
        UINT32 packetLength = 0;
        HRESULT hr = captureClient->GetNextPacketSize(&packetLength);
        
        if (FAILED(hr)) {
            std::cerr << "Failed to get packet size: 0x" << std::hex << hr << std::endl;
            break;
        }

        while (packetLength != 0) {
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = captureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr)) {
                std::cerr << "Failed to get buffer: 0x" << std::hex << hr << std::endl;
                break;
            }

            ProcessAudioData(pData, numFramesAvailable, flags);

            hr = captureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) {
                std::cerr << "Failed to release buffer: 0x" << std::hex << hr << std::endl;
                break;
            }

            hr = captureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) {
                break;
            }
        }

        // Update capture time
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        {
            std::lock_guard<std::mutex> lock(statsMutex);
            stats.captureTimeSeconds = duration.count() / 1000.0;
        }

        // Small sleep to avoid consuming too much CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void AudioCapture::ProcessAudioData(BYTE* audioData, UINT32 numFrames, DWORD flags) {
    if (!audioData || numFrames == 0) {
        return;
    }

    // Calculate the number of bytes
    UINT32 bytesPerFrame = currentFormat.channels * (currentFormat.bitsPerSample / 8);
    UINT32 totalBytes = numFrames * bytesPerFrame;

    // Handle silence flag
    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
        // Buffer contains silence, we can either skip it or fill with zeros
        audioBuffer.assign(totalBytes, 0);
    } else {
        // Copy audio data to our buffer
        audioBuffer.assign(audioData, audioData + totalBytes);
    }

    // Update statistics
    UpdateStats(numFrames, totalBytes);

    // Call the callback if set
    if (audioCallback) {
        audioCallback(audioBuffer, currentFormat);
    }
}

void AudioCapture::UpdateStats(UINT32 framesProcessed, UINT32 bytesProcessed) {
    std::lock_guard<std::mutex> lock(statsMutex);
    stats.totalFramesCaptured += framesProcessed;
    stats.totalBytesProcessed += bytesProcessed;
}

void AudioCapture::Cleanup() {
    if (captureClient) {
        captureClient->Release();
        captureClient = nullptr;
    }

    if (audioClient) {
        audioClient->Release();
        audioClient = nullptr;
    }

    if (audioDevice) {
        audioDevice->Release();
        audioDevice = nullptr;
    }

    if (deviceEnumerator) {
        deviceEnumerator->Release();
        deviceEnumerator = nullptr;
    }

    if (waveFormat) {
        CoTaskMemFree(waveFormat);
        waveFormat = nullptr;
    }
}