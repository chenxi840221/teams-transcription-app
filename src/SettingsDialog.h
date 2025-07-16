#pragma once

#include <windows.h>
#include <commctrl.h>
#include <string>
#include "ConfigManager.h"
#include "SpeechRecognition.h"

class SettingsDialog {
public:
    SettingsDialog(HINSTANCE hInstance, HWND hParent);
    ~SettingsDialog();

    // Show the dialog and return true if OK was clicked
    bool ShowDialog(ConfigManager* configManager);

private:
    HINSTANCE hInstance;
    HWND hParent;
    HWND hDialog;
    ConfigManager* configManager;
    
    // Dialog procedure
    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Initialize dialog controls
    void InitializeDialog();
    void LoadSettings();
    void SaveSettings();
    
    // Control handlers
    void OnProviderChanged();
    void OnTestConnection();
    void UpdateControlStates();
    
    // Helper methods
    void PopulateProviderCombo();
    void PopulateLanguageCombo();
    void PopulateAudioQualityCombo();
    std::string GetSelectedProvider();
    void SetSelectedProvider(const std::string& provider);
    
    // Test connection to AI services
    bool TestAzureConnection(const std::string& apiKey, const std::string& region);
    bool TestGoogleConnection(const std::string& apiKey);
    bool TestOpenAIConnection(const std::string& apiKey);
    bool TestAzureOpenAIConnection(const std::string& apiKey, const std::string& endpoint);
};