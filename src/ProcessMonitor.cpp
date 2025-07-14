#include "ProcessMonitor.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <algorithm>
#include <vector>

#pragma comment(lib, "psapi.lib")

ProcessMonitor::ProcessMonitor()
    : isMonitoring(false)
    , teamsProcessId(0)
    , lastMeetingStatus(false)
{
}

ProcessMonitor::~ProcessMonitor() {
    StopMonitoring();
}

bool ProcessMonitor::StartMonitoring() {
    if (isMonitoring.load()) {
        return true; // Already monitoring
    }

    isMonitoring.store(true);
    monitoringThread = std::thread(&ProcessMonitor::MonitoringThreadProc, this);
    
    std::cout << "Process monitoring started" << std::endl;
    return true;
}

void ProcessMonitor::StopMonitoring() {
    if (!isMonitoring.load()) {
        return; // Not monitoring
    }

    isMonitoring.store(false);
    
    if (monitoringThread.joinable()) {
        monitoringThread.join();
    }

    std::cout << "Process monitoring stopped" << std::endl;
}

bool ProcessMonitor::IsTeamsRunning() const {
    return teamsProcessId != 0 && FindTeamsProcesses();
}

bool ProcessMonitor::IsTeamsInMeeting() const {
    return lastMeetingStatus;
}

void ProcessMonitor::SetTeamsStatusCallback(TeamsStatusCallback callback) {
    statusCallback = callback;
}

std::vector<ProcessMonitor::ProcessInfo> ProcessMonitor::GetTeamsProcesses() const {
    std::vector<ProcessInfo> processes;
    
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return processes;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hProcessSnap, &pe32)) {
        do {
            std::wstring processName(pe32.szExeFile);
            
            // Look for Teams-related processes
            if (processName.find(L"Teams.exe") != std::wstring::npos ||
                processName.find(L"ms-teams.exe") != std::wstring::npos ||
                processName.find(L"TeamsMeetingAddin") != std::wstring::npos) {
                
                ProcessInfo info;
                info.processId = pe32.th32ProcessID;
                info.processName = processName;
                info.isInMeeting = false; // Will be determined by window analysis

                // Try to get the main window title
                EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&info));
                
                processes.push_back(info);
            }
        } while (Process32NextW(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
    return processes;
}

void ProcessMonitor::MonitoringThreadProc() {
    while (isMonitoring.load()) {
        bool teamsFound = FindTeamsProcesses();
        bool inMeeting = CheckMeetingStatus();
        
        // Notify if status changed
        if (statusCallback && (inMeeting != lastMeetingStatus || teamsFound != (teamsProcessId != 0))) {
            std::string meetingInfo;
            if (inMeeting) {
                meetingInfo = "Active meeting detected";
            }
            statusCallback(inMeeting, meetingInfo);
        }
        
        lastMeetingStatus = inMeeting;
        
        // Sleep for 2 seconds before next check
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

bool ProcessMonitor::FindTeamsProcesses() {
    teamsProcessId = 0;
    
    auto processes = GetTeamsProcesses();
    
    if (!processes.empty()) {
        // Use the first Teams process found
        teamsProcessId = processes[0].processId;
        return true;
    }
    
    return false;
}

bool ProcessMonitor::CheckMeetingStatus() {
    if (teamsProcessId == 0) {
        return false;
    }

    // Check if Teams is in a meeting by analyzing window titles
    struct WindowData {
        DWORD processId;
        bool inMeeting;
        std::wstring windowTitle;
    };

    WindowData data;
    data.processId = teamsProcessId;
    data.inMeeting = false;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        WindowData* data = reinterpret_cast<WindowData*>(lParam);
        
        DWORD windowProcessId;
        GetWindowThreadProcessId(hwnd, &windowProcessId);
        
        if (windowProcessId == data->processId) {
            wchar_t windowTitle[256];
            GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));
            
            std::wstring title(windowTitle);
            data->windowTitle = title;
            
            // Check for meeting indicators in window title
            if (title.find(L"Meeting") != std::wstring::npos ||
                title.find(L"Call") != std::wstring::npos ||
                title.find(L"Conversation") != std::wstring::npos ||
                title.find(L"- Microsoft Teams") != std::wstring::npos) {
                
                // Additional checks for meeting state
                if (title.find(L"Muted") != std::wstring::npos ||
                    title.find(L"Unmuted") != std::wstring::npos ||
                    title.find(L"Calling") != std::wstring::npos ||
                    title.find(L"In call") != std::wstring::npos) {
                    data->inMeeting = true;
                    return FALSE; // Stop enumeration
                }
            }
        }
        
        return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&data));

    return data.inMeeting;
}

std::wstring ProcessMonitor::GetWindowTitle(HWND hwnd) {
    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
    return std::wstring(title);
}

BOOL CALLBACK ProcessMonitor::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    ProcessInfo* info = reinterpret_cast<ProcessInfo*>(lParam);
    
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    
    if (windowProcessId == info->processId) {
        // Check if this is a visible window
        if (IsWindowVisible(hwnd)) {
            wchar_t windowTitle[256];
            GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));
            
            std::wstring title(windowTitle);
            if (!title.empty()) {
                info->windowTitle = title;
                
                // Check for meeting indicators
                if (title.find(L"Meeting") != std::wstring::npos ||
                    title.find(L"Call") != std::wstring::npos ||
                    title.find(L"Muted") != std::wstring::npos ||
                    title.find(L"Unmuted") != std::wstring::npos) {
                    info->isInMeeting = true;
                }
                
                return FALSE; // Found main window, stop enumeration
            }
        }
    }
    
    return TRUE; // Continue enumeration
}