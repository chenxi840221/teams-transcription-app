#include "MainWindow.h"
#include "AudioCapture.h"
#include "ProcessMonitor.h"
#include "SpeechRecognition.h"
#include "ConfigManager.h"
#include "SettingsDialog.h"
#include "SimpleLogger.h"
#include "resource.h"
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <iostream>
#include <sstream>

#pragma comment(lib, "shell32.lib")

const wchar_t* WINDOW_CLASS_NAME = L"TeamsTranscriptionMainWindow";
const int TIMER_UPDATE_STATS = 1;
const int TIMER_AUTO_SAVE = 2;

// Control IDs
enum ControlIds {
    ID_START_BUTTON = 1001,
    ID_STOP_BUTTON = 1002,
    ID_PAUSE_BUTTON = 1003,
    ID_SETTINGS_BUTTON = 1004,
    ID_EXPORT_BUTTON = 1005,
    ID_CLEAR_BUTTON = 1006,
    ID_TRANSCRIPTION_EDIT = 1007,
    ID_DEBUG_LOG_EDIT = 1008,
    ID_STATUS_BAR = 1009,
    ID_TEAMS_STATUS = 1010,
    ID_PROGRESS_BAR = 1011
};

// System tray
const UINT WM_TRAYICON = WM_USER + 1;
const UINT TRAY_ICON_ID = 1;

MainWindow::MainWindow()
    : hwnd(nullptr)
    , hInstance(nullptr)
    , isRecording(false)
    , isPaused(false)
{
    memset(&notifyIconData, 0, sizeof(notifyIconData));
}

MainWindow::~MainWindow() {
    if (notifyIconData.hWnd) {
        Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
    }
}

bool MainWindow::Create(HINSTANCE hInst, int nCmdShow) {
    hInstance = hInst;

    if (!RegisterWindowClass()) {
        return false;
    }

    // Create main window (increased width for dual panels)
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        WINDOW_CLASS_NAME,
        L"Teams Audio Transcription",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 650,
        nullptr, nullptr, hInstance, this);

    if (!hwnd) {
        return false;
    }

    if (!InitializeComponents()) {
        return false;
    }

    CreateControls();
    SetupSystemTray();

    // Force initial resize to apply dynamic layout
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    HandleResize(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    // Add initial debug messages after controls are created
    UpdateDebugLog("Application initialized successfully");
    UpdateDebugLog("Audio capture: " + std::string(audioCapture ? "Ready" : "Failed"));
    UpdateDebugLog("Speech recognition: " + std::string(speechRecognition ? "Ready" : "Failed"));

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Start update timers
    SetTimer(hwnd, TIMER_UPDATE_STATS, 1000, nullptr);  // Update every second
    SetTimer(hwnd, TIMER_AUTO_SAVE, 60000, nullptr);    // Auto-save every minute

    return true;
}

int MainWindow::MessageLoop() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

bool MainWindow::RegisterWindowClass() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    return RegisterClassEx(&wc) != 0;
}

bool MainWindow::InitializeComponents() {
    try {
        // Initialize debug logger first
        SimpleLogger::Initialize();
        INFO_LOG("=== TEAMS TRANSCRIPTION APP STARTING ===");
        
        configManager = std::make_unique<ConfigManager>();
        bool configLoaded = configManager->LoadConfig();
        INFO_LOG("Config loaded: " + std::string(configLoaded ? "SUCCESS" : "FAILED"));
        
        if (configLoaded) {
            auto config = configManager->GetConfig();
            CONFIG_LOG("Provider", std::to_string((int)config.speechConfig.provider));
            std::string apiKeyStatus = config.speechConfig.apiKey.empty() ? "EMPTY" : "SET (" + std::to_string(config.speechConfig.apiKey.length()) + " chars)";
            CONFIG_LOG("API Key", apiKeyStatus);
            CONFIG_LOG("Endpoint", config.speechConfig.endpoint);
            CONFIG_LOG("Language", config.speechConfig.language);
        }
        
        settingsDialog = std::make_unique<SettingsDialog>(hInstance, hwnd);

        audioCapture = std::make_unique<AudioCapture>();
        HRESULT hr = audioCapture->Initialize();
        if (FAILED(hr)) {
            MessageBox(hwnd, L"Failed to initialize audio capture", L"Error", MB_OK | MB_ICONERROR);
            return false;
        }

        speechRecognition = std::make_unique<SpeechRecognition>();
        auto speechConfig = configManager->GetSpeechConfig();
        
        // Debug output
        std::cout << "Initializing speech recognition with provider: " << (int)speechConfig.provider << std::endl;
        std::cout << "Language: " << speechConfig.language << std::endl;
        
        bool initResult = speechRecognition->Initialize(speechConfig);
        std::cout << "Speech recognition initialized: " << (initResult ? "SUCCESS" : "FAILED") << std::endl;

        processMonitor = std::make_unique<ProcessMonitor>();
        processMonitor->SetTeamsStatusCallback([this](bool isInMeeting, const std::string& meetingInfo) {
            UpdateTeamsStatus(isInMeeting, meetingInfo);
        });

        // Set up audio data callback
        audioCapture->SetAudioDataCallback([this](const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) {
            ProcessAudioData(audioData, format);
        });

        // Set up transcription callback
        speechRecognition->SetTranscriptionCallback([this](const std::string& text, double confidence) {
            UpdateTranscription(text, confidence);
        });

        return true;
    }
    catch (const std::exception& e) {
        std::wstring error = L"Failed to initialize components: ";
        error += std::wstring(e.what(), e.what() + strlen(e.what()));
        MessageBox(hwnd, error.c_str(), L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
}

void MainWindow::CreateControls() {
    // Control buttons (top row)
    CreateWindow(L"BUTTON", L"Start Recording", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10, 10, 100, 30, hwnd, (HMENU)ID_START_BUTTON, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Stop Recording", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        120, 10, 100, 30, hwnd, (HMENU)ID_STOP_BUTTON, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Pause", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
        230, 10, 80, 30, hwnd, (HMENU)ID_PAUSE_BUTTON, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Settings", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        320, 10, 80, 30, hwnd, (HMENU)ID_SETTINGS_BUTTON, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Export", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        410, 10, 80, 30, hwnd, (HMENU)ID_EXPORT_BUTTON, hInstance, nullptr);

    CreateWindow(L"BUTTON", L"Clear", 
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        500, 10, 80, 30, hwnd, (HMENU)ID_CLEAR_BUTTON, hInstance, nullptr);

    // Teams status label
    CreateWindow(L"STATIC", L"Teams Status: Not Connected", 
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        10, 50, 300, 20, hwnd, (HMENU)ID_TEAMS_STATUS, hInstance, nullptr);

    // Progress bar
    CreateWindow(PROGRESS_CLASS, nullptr,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        320, 50, 200, 20, hwnd, (HMENU)ID_PROGRESS_BAR, hInstance, nullptr);

    // Transcription area (top half) - dynamically resized in HandleResize
    CreateWindow(L"EDIT", L"Transcription will appear here...",
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 105, 980, 200, hwnd, (HMENU)ID_TRANSCRIPTION_EDIT, hInstance, nullptr);
        
    // Debug log area (bottom half) - dynamically resized in HandleResize  
    CreateWindow(L"EDIT", L"Debug information will appear here...",
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 370, 980, 200, hwnd, (HMENU)ID_DEBUG_LOG_EDIT, hInstance, nullptr);

    // Status bar
    CreateWindow(STATUSCLASSNAME, L"Ready",
        WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
        0, 0, 0, 0, hwnd, (HMENU)ID_STATUS_BAR, hInstance, nullptr);
}

void MainWindow::SetupSystemTray() {
    notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    notifyIconData.hWnd = hwnd;
    notifyIconData.uID = TRAY_ICON_ID;
    notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    notifyIconData.uCallbackMessage = WM_TRAYICON;
    notifyIconData.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY_ICON));
    wcscpy_s(notifyIconData.szTip, L"Teams Transcription");

    Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            return HandleCommand(LOWORD(wParam), HIWORD(wParam));

        case WM_TIMER:
            return HandleTimer(wParam);

        case WM_TRAYICON:
            return HandleTrayMessage(lParam);

        case WM_SIZE:
            HandleResize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_SYSCOMMAND:
            if (wParam == SC_MINIMIZE && configManager && configManager->GetConfig().minimizeToTray) {
                ShowWindow(hwnd, SW_HIDE);
                return 0;
            }
            break;

        case WM_CLOSE:
            if (isRecording.load()) {
                int result = MessageBox(hwnd, L"Recording is in progress. Stop recording before closing?", 
                                      L"Confirm Close", MB_YESNOCANCEL | MB_ICONQUESTION);
                if (result == IDYES) {
                    StopRecording();
                } else if (result == IDCANCEL) {
                    return 0;
                }
            }
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleCommand(WORD commandId, WORD notificationCode) {
    switch (commandId) {
        case ID_START_BUTTON:
            StartRecording();
            break;

        case ID_STOP_BUTTON:
            StopRecording();
            break;

        case ID_PAUSE_BUTTON:
            TogglePause();
            break;

        case ID_SETTINGS_BUTTON:
            ShowSettingsDialog();
            break;

        case ID_EXPORT_BUTTON:
            ExportTranscription();
            break;

        case ID_CLEAR_BUTTON:
            ClearTranscription();
            break;
    }
    return 0;
}

LRESULT MainWindow::HandleTimer(WPARAM timerId) {
    switch (timerId) {
        case TIMER_UPDATE_STATS:
            UpdateCaptureStats();
            break;

        case TIMER_AUTO_SAVE:
            if (isRecording.load()) {
                AutoSaveTranscription();
            }
            break;
    }
    return 0;
}

LRESULT MainWindow::HandleTrayMessage(LPARAM lParam) {
    switch (lParam) {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            break;

        case WM_RBUTTONUP:
            // Could show context menu here
            break;
    }
    return 0;
}

void MainWindow::HandleResize(int width, int height) {
    // Resize status bar
    HWND statusBar = GetDlgItem(hwnd, ID_STATUS_BAR);
    if (statusBar) {
        SendMessage(statusBar, WM_SIZE, 0, 0);
    }

    // Calculate available content area
    RECT statusRect;
    GetWindowRect(statusBar, &statusRect);
    int statusHeight = statusRect.bottom - statusRect.top;
    int availableHeight = height - 120 - statusHeight; // Total content height
    int halfHeight = availableHeight / 2; // Half height for each section
    
    // Resize transcription edit control (TOP HALF)
    HWND transcriptionControl = GetDlgItem(hwnd, ID_TRANSCRIPTION_EDIT);
    if (transcriptionControl) {
        SetWindowPos(transcriptionControl, nullptr, 10, 105, width - 20, halfHeight - 30, 
                    SWP_NOZORDER);
    }
    
    // Position separator line (MIDDLE)
    int separatorY = 105 + halfHeight - 20;
    // Note: Separator will be repositioned in CreateControls, this is for reference
    
    // Resize debug log edit control (BOTTOM HALF)
    HWND debugLogControl = GetDlgItem(hwnd, ID_DEBUG_LOG_EDIT);
    if (debugLogControl) {
        int debugY = 105 + halfHeight + 10; // Start after transcription + separator space
        SetWindowPos(debugLogControl, nullptr, 10, debugY, width - 20, halfHeight - 30, 
                    SWP_NOZORDER);
    }
}

void MainWindow::StartRecording() {
    if (isRecording.load()) {
        return;
    }

    // Check for consent if required
    if (configManager && configManager->GetConfig().requireConsent) {
        int result = MessageBox(hwnd, 
            L"This application will record system audio. Ensure you have consent from all meeting participants. Continue?",
            L"Recording Consent", MB_YESNO | MB_ICONWARNING);
        if (result != IDYES) {
            return;
        }
    }

    if (audioCapture) {
        HRESULT hr = audioCapture->StartCapture();
        if (SUCCEEDED(hr)) {
            isRecording.store(true);
            isPaused.store(false);

            // Update UI
            EnableWindow(GetDlgItem(hwnd, ID_START_BUTTON), FALSE);
            EnableWindow(GetDlgItem(hwnd, ID_STOP_BUTTON), TRUE);
            EnableWindow(GetDlgItem(hwnd, ID_PAUSE_BUTTON), TRUE);

            SetWindowText(GetDlgItem(hwnd, ID_STATUS_BAR), L"Recording...");
            
            // Add GUI debug message
            UpdateDebugLog("Audio capture started successfully - listening for audio");

            // Start process monitoring
            if (processMonitor) {
                processMonitor->StartMonitoring();
            }
        } else {
            MessageBox(hwnd, L"Failed to start audio capture", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::StopRecording() {
    if (!isRecording.load()) {
        return;
    }

    if (audioCapture) {
        audioCapture->StopCapture();
    }

    if (processMonitor) {
        processMonitor->StopMonitoring();
    }

    isRecording.store(false);
    isPaused.store(false);

    // Update UI
    EnableWindow(GetDlgItem(hwnd, ID_START_BUTTON), TRUE);
    EnableWindow(GetDlgItem(hwnd, ID_STOP_BUTTON), FALSE);
    EnableWindow(GetDlgItem(hwnd, ID_PAUSE_BUTTON), FALSE);

    SetWindowText(GetDlgItem(hwnd, ID_STATUS_BAR), L"Stopped");
}

void MainWindow::TogglePause() {
    // For now, this is a simple implementation
    // In a full implementation, this would pause/resume audio processing
    isPaused.store(!isPaused.load());
    
    if (isPaused.load()) {
        SetWindowText(GetDlgItem(hwnd, ID_PAUSE_BUTTON), L"Resume");
        SetWindowText(GetDlgItem(hwnd, ID_STATUS_BAR), L"Paused");
    } else {
        SetWindowText(GetDlgItem(hwnd, ID_PAUSE_BUTTON), L"Pause");
        SetWindowText(GetDlgItem(hwnd, ID_STATUS_BAR), L"Recording...");
    }
}

void MainWindow::ShowSettingsDialog() {
    if (settingsDialog && configManager) {
        if (settingsDialog->ShowDialog(configManager.get())) {
            // Settings were changed, reinitialize speech recognition
            auto speechConfig = configManager->GetSpeechConfig();
            
            std::cout << "Settings changed, reinitializing speech recognition..." << std::endl;
            std::cout << "New provider: " << (int)speechConfig.provider << std::endl;
            
            if (speechRecognition) {
                speechRecognition->Initialize(speechConfig);
                
                // Re-set the callback
                speechRecognition->SetTranscriptionCallback([this](const std::string& text, double confidence) {
                    UpdateTranscription(text, confidence);
                });
            }
        }
    } else {
        MessageBox(hwnd, L"Settings dialog not available", L"Settings", MB_OK | MB_ICONWARNING);
    }
}

void MainWindow::ExportTranscription() {
    MessageBox(hwnd, L"Export functionality not implemented yet", L"Export", MB_OK | MB_ICONINFORMATION);
}

void MainWindow::ClearTranscription() {
    SetWindowText(GetDlgItem(hwnd, ID_TRANSCRIPTION_EDIT), L"");
    SetWindowText(GetDlgItem(hwnd, ID_DEBUG_LOG_EDIT), L"");
}

void MainWindow::AutoSaveTranscription() {
    // Auto-save implementation would go here
}

void MainWindow::ProcessAudioData(const std::vector<BYTE>& audioData, const AudioCapture::AudioFormat& format) {
    static int audioCallCount = 0;
    audioCallCount++;
    
    // Log detailed audio information
    AUDIO_LOG("MainWindow", audioData.size(), 
              "Rate: " + std::to_string(format.sampleRate) + "Hz, " +
              "Channels: " + std::to_string(format.channels) + ", " +
              "Bits: " + std::to_string(format.bitsPerSample));
    
    if (audioCallCount % 100 == 0) { // Log summary every 100th call
        INFO_LOG("ProcessAudioData called " + std::to_string(audioCallCount) + " times, latest size: " + std::to_string(audioData.size()) + " bytes");
    }
    
    // Log audio samples for debugging (every 50th call to avoid spam)
    if (audioCallCount % 50 == 0) {
        DEBUG_LOG("Audio sample #" + std::to_string(audioCallCount) + " - " + std::to_string(audioData.size()) + " bytes");
    }
    
    if (isPaused.load() || !speechRecognition) {
        if (audioCallCount % 100 == 0) {
            WARN_LOG("Audio processing skipped - paused: " + std::string(isPaused.load() ? "true" : "false") + 
                    ", speechRecognition: " + (speechRecognition ? "valid" : "null"));
        }
        return;
    }

    DEBUG_LOG("Forwarding audio to speech recognition, size: " + std::to_string(audioData.size()));
    
    // Forward audio data to speech recognition
    speechRecognition->ProcessAudioData(audioData, format);
}

void MainWindow::UpdateTranscription(const std::string& text, double confidence) {
    INFO_LOG("UpdateTranscription called with text: '" + text + "', confidence: " + std::to_string(confidence));
    
    if (text.empty()) {
        WARN_LOG("UpdateTranscription received empty text, skipping");
        return;
    }

    // Check if this is a demo/debug message - filter these out
    if ((text.find("Azure OpenAI GPT-4o transcription #") != std::string::npos &&
         text.find("Processed") != std::string::npos &&
         text.find("bytes of WAV audio") != std::string::npos) ||
        (text.find("Demo transcription #") != std::string::npos &&
         text.find("Audio detected") != std::string::npos)) {
        // This is debug info, send to debug log instead
        UpdateDebugLog("Demo/Debug message filtered: " + text);
        INFO_LOG("TRANSCRIPTION FILTER: Demo message filtered and sent to debug log");
        return;
    }

    INFO_LOG("TRANSCRIPTION: Adding to main panel: '" + text + "'");

    // Convert to wide string for Windows controls
    int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring wText(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wText[0], len);

    // Append to transcription edit control (clean transcription only)
    HWND editControl = GetDlgItem(hwnd, ID_TRANSCRIPTION_EDIT);
    if (editControl) {
        // Get current text length and move cursor to end
        int textLength = GetWindowTextLength(editControl);
        SendMessage(editControl, EM_SETSEL, textLength, textLength);
        
        // Add text without timestamp for clean transcription
        std::wstring cleanText = wText + L"\r\n";
        SendMessage(editControl, EM_REPLACESEL, FALSE, (LPARAM)cleanText.c_str());
        
        // Scroll to bottom
        SendMessage(editControl, EM_SCROLLCARET, 0, 0);
        
        INFO_LOG("Transcription added to UI successfully");
    } else {
        ERROR_LOG("Failed to get transcription edit control handle");
    }
}

void MainWindow::UpdateDebugLog(const std::string& debugInfo) {
    if (debugInfo.empty()) {
        return;
    }

    // Convert to wide string for Windows controls
    int len = MultiByteToWideChar(CP_UTF8, 0, debugInfo.c_str(), -1, nullptr, 0);
    std::wstring wText(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, debugInfo.c_str(), -1, &wText[0], len);

    // Append to debug log edit control
    HWND debugControl = GetDlgItem(hwnd, ID_DEBUG_LOG_EDIT);
    if (debugControl) {
        // Get current text length and move cursor to end
        int textLength = GetWindowTextLength(debugControl);
        SendMessage(debugControl, EM_SETSEL, textLength, textLength);
        
        // Add timestamp and debug info
        std::wstring timestampedText = L"[" + std::to_wstring(GetTickCount() / 1000) + L"s] " + wText + L"\r\n";
        SendMessage(debugControl, EM_REPLACESEL, FALSE, (LPARAM)timestampedText.c_str());
        
        // Scroll to bottom
        SendMessage(debugControl, EM_SCROLLCARET, 0, 0);
    }
}

void MainWindow::UpdateTeamsStatus(bool isInMeeting, const std::string& meetingInfo) {
    std::string updateMsg = "MainWindow: UpdateTeamsStatus called - isInMeeting: " + 
                          std::string(isInMeeting ? "YES" : "NO") + 
                          ", meetingInfo: " + meetingInfo;
    INFO_LOG(updateMsg);
    
    std::wstring status = L"Teams Status: ";
    if (isInMeeting) {
        status += L"In Meeting";
    } else if (meetingInfo.find("Teams detected") != std::string::npos) {
        status += L"Connected (Not in Meeting)";
    } else {
        status += L"Not Connected";
    }
    
    if (!meetingInfo.empty()) {
        int len = MultiByteToWideChar(CP_UTF8, 0, meetingInfo.c_str(), -1, nullptr, 0);
        std::wstring wMeetingInfo(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, meetingInfo.c_str(), -1, &wMeetingInfo[0], len);
        // Only show detailed info for debugging, not in UI
    }
    
    HWND teamsStatusControl = GetDlgItem(hwnd, ID_TEAMS_STATUS);
    if (teamsStatusControl) {
        SetWindowText(teamsStatusControl, status.c_str());
        // Convert wide string to string for logging
        std::string statusStr(status.begin(), status.end());
        std::string successMsg = "MainWindow: Teams status updated to: " + statusStr;
        INFO_LOG(successMsg);
    } else {
        ERROR_LOG("MainWindow: ERROR - Teams status control not found!");
    }
}

void MainWindow::UpdateCaptureStats() {
    if (!audioCapture) {
        return;
    }

    auto stats = audioCapture->GetCaptureStats();
    
    std::wstringstream statusText;
    statusText << L"Captured: " << stats.totalFramesCaptured << L" frames, "
               << L"Time: " << static_cast<int>(stats.captureTimeSeconds) << L"s";

    SetWindowText(GetDlgItem(hwnd, ID_STATUS_BAR), statusText.str().c_str());
}