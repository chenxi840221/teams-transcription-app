#pragma once

#include <windows.h>
#include <memory>
#include <string>
#include <atomic>
#include <shellapi.h>

class AudioCapture;
class ProcessMonitor;
class SpeechRecognition;
class ConfigManager;

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool Create(HINSTANCE hInstance, int nCmdShow);
    int MessageLoop();

private:
    HWND hwnd;
    HINSTANCE hInstance;
    
    std::unique_ptr<AudioCapture> audioCapture;
    std::unique_ptr<ProcessMonitor> processMonitor;
    std::unique_ptr<SpeechRecognition> speechRecognition;
    std::unique_ptr<ConfigManager> configManager;

    std::atomic<bool> isRecording;
    std::atomic<bool> isPaused;
    
    NOTIFYICONDATA notifyIconData;

    bool RegisterWindowClass();
    bool InitializeComponents();
    void CreateControls();
    void SetupSystemTray();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleCommand(WORD commandId, WORD notificationCode);
    LRESULT HandleTimer(WPARAM timerId);
    LRESULT HandleTrayMessage(LPARAM lParam);
    void HandleResize(int width, int height);

    void StartRecording();
    void StopRecording();
    void TogglePause();
    void ShowSettingsDialog();
    void ExportTranscription();
    void ClearTranscription();
    void AutoSaveTranscription();

    void ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format);
    void UpdateTranscription(const std::string& text, double confidence);
    void UpdateTeamsStatus(bool isInMeeting, const std::string& meetingInfo);
    void UpdateCaptureStats();
};
