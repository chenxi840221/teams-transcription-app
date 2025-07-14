#include "MainWindow.h"
#include "ConfigManager.h"
#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        MessageBox(nullptr, L"Failed to initialize COM library", L"Error", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    // Load configuration
    ConfigManager config;
    if (!config.LoadConfig()) {
        MessageBox(nullptr, L"Failed to load configuration", L"Warning", MB_OK | MB_ICONWARNING);
    }

    // Create and run main window
    MainWindow mainWindow;
    if (!mainWindow.Create(hInstance, nCmdShow)) {
        MessageBox(nullptr, L"Failed to create main window", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return -1;
    }

    int result = mainWindow.MessageLoop();

    CoUninitialize();
    return result;
}
