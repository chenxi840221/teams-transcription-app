#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>

class ProcessMonitor {
public:
    using TeamsStatusCallback = std::function<void(bool isInMeeting, const std::string& meetingInfo)>;

    ProcessMonitor();
    ~ProcessMonitor();

    bool StartMonitoring();
    void StopMonitoring();
    bool IsTeamsRunning() const;
    bool IsTeamsInMeeting() const;
    void SetTeamsStatusCallback(TeamsStatusCallback callback);

    struct ProcessInfo {
        DWORD processId;
        std::wstring processName;
        std::wstring windowTitle;
        bool isInMeeting;
    };

    std::vector<ProcessInfo> GetTeamsProcesses() const;

private:
    std::atomic<bool> isMonitoring;
    std::thread monitoringThread;
    TeamsStatusCallback statusCallback;

    mutable DWORD teamsProcessId;
    mutable bool lastMeetingStatus;

    void MonitoringThreadProc();
    bool FindTeamsProcesses();
    bool CheckMeetingStatus();
    std::wstring GetWindowTitle(HWND hwnd);
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
};
