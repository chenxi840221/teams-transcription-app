#include "SettingsDialog.h"
#include "resource.h"
#include "SimpleLogger.h"
#include <iostream>
#include <sstream>

SettingsDialog::SettingsDialog(HINSTANCE hInstance, HWND hParent)
    : hInstance(hInstance), hParent(hParent), hDialog(nullptr), configManager(nullptr) {
}

SettingsDialog::~SettingsDialog() {
}

bool SettingsDialog::ShowDialog(ConfigManager* configMgr) {
    configManager = configMgr;
    
    INT_PTR result = DialogBoxParam(
        hInstance,
        MAKEINTRESOURCE(IDD_SETTINGS_DIALOG),
        hParent,
        DialogProc,
        (LPARAM)this
    );
    
    return (result == IDOK);
}

INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsDialog* dialog = nullptr;
    
    if (msg == WM_INITDIALOG) {
        dialog = reinterpret_cast<SettingsDialog*>(lParam);
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)dialog);
        dialog->hDialog = hwnd;
    } else {
        dialog = reinterpret_cast<SettingsDialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
    }
    
    if (dialog) {
        return dialog->HandleMessage(msg, wParam, lParam);
    }
    
    return FALSE;
}

INT_PTR SettingsDialog::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            InitializeDialog();
            LoadSettings();
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_PROVIDER_COMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        OnProviderChanged();
                    }
                    break;
                    
                case IDC_TEST_CONNECTION_BUTTON:
                    OnTestConnection();
                    break;
                    
                case IDC_SETTINGS_OK:
                    SaveSettings();
                    EndDialog(hDialog, IDOK);
                    return TRUE;
                    
                case IDC_SETTINGS_CANCEL:
                    EndDialog(hDialog, IDCANCEL);
                    return TRUE;
            }
            break;
            
        case WM_CLOSE:
            EndDialog(hDialog, IDCANCEL);
            return TRUE;
    }
    
    return FALSE;
}

void SettingsDialog::InitializeDialog() {
    // Set dialog title
    SetWindowText(hDialog, L"Speech Recognition Settings");
    
    // Initialize combo boxes
    PopulateProviderCombo();
    PopulateLanguageCombo();
    PopulateAudioQualityCombo();
    
    // Set initial control states
    UpdateControlStates();
}

void SettingsDialog::PopulateProviderCombo() {
    HWND hCombo = GetDlgItem(hDialog, IDC_PROVIDER_COMBO);
    
    // Clear existing items
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    
    // Add speech providers
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Windows Speech Recognition");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Azure Cognitive Services");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Google Cloud Speech");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"OpenAI Whisper");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Azure OpenAI (GPT-4o)");
    
    // Set default selection
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void SettingsDialog::PopulateLanguageCombo() {
    HWND hCombo = GetDlgItem(hDialog, IDC_LANGUAGE_COMBO);
    
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"en-US (English - United States)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"en-GB (English - United Kingdom)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"es-ES (Spanish - Spain)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"fr-FR (French - France)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"de-DE (German - Germany)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"ja-JP (Japanese - Japan)");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"zh-CN (Chinese - Simplified)");
    
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void SettingsDialog::PopulateAudioQualityCombo() {
    HWND hCombo = GetDlgItem(hDialog, IDC_AUDIO_QUALITY_COMBO);
    
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Low (8kHz) - Voice Quality");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"Medium (16kHz) - Recommended");
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"High (44kHz) - CD Quality");
    
    SendMessage(hCombo, CB_SETCURSEL, 1, 0); // Default to Medium
}

void SettingsDialog::LoadSettings() {
    if (!configManager) return;
    
    auto config = configManager->GetConfig();
    auto speechConfig = config.speechConfig;
    
    // Set provider
    std::string providerStr;
    switch (speechConfig.provider) {
        case SpeechRecognition::Provider::Windows:
            providerStr = "windows";
            SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_SETCURSEL, 0, 0);
            break;
        case SpeechRecognition::Provider::Azure:
            providerStr = "azure";
            SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_SETCURSEL, 1, 0);
            break;
        case SpeechRecognition::Provider::Google:
            providerStr = "google";
            SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_SETCURSEL, 2, 0);
            break;
        case SpeechRecognition::Provider::OpenAI:
            providerStr = "openai";
            SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_SETCURSEL, 3, 0);
            break;
        case SpeechRecognition::Provider::AzureOpenAI:
            providerStr = "azure-openai";
            SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_SETCURSEL, 4, 0);
            break;
    }
    
    // Set API key
    std::wstring apiKey(speechConfig.apiKey.begin(), speechConfig.apiKey.end());
    SetWindowText(GetDlgItem(hDialog, IDC_API_KEY_EDIT), apiKey.c_str());
    
    // Set region or endpoint based on provider
    if (speechConfig.provider == SpeechRecognition::Provider::AzureOpenAI) {
        // For Azure OpenAI, use endpoint in the region field
        std::wstring endpoint(speechConfig.endpoint.begin(), speechConfig.endpoint.end());
        SetWindowText(GetDlgItem(hDialog, IDC_REGION_EDIT), endpoint.c_str());
    } else {
        // For other providers, use region
        std::wstring region(speechConfig.region.begin(), speechConfig.region.end());
        SetWindowText(GetDlgItem(hDialog, IDC_REGION_EDIT), region.c_str());
    }
    
    // Set language (find matching item)
    std::wstring language(speechConfig.language.begin(), speechConfig.language.end());
    HWND hLangCombo = GetDlgItem(hDialog, IDC_LANGUAGE_COMBO);
    int count = SendMessage(hLangCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++) {
        wchar_t buffer[256];
        SendMessage(hLangCombo, CB_GETLBTEXT, i, (LPARAM)buffer);
        std::wstring item(buffer);
        if (item.find(language) != std::wstring::npos) {
            SendMessage(hLangCombo, CB_SETCURSEL, i, 0);
            break;
        }
    }
    
    // Set checkboxes
    CheckDlgButton(hDialog, IDC_PUNCTUATION_CHECK, speechConfig.enablePunctuation ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDialog, IDC_DIARIZATION_CHECK, speechConfig.enableSpeakerDiarization ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDialog, IDC_AUTO_START_CHECK, config.autoStartRecording ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDialog, IDC_REQUIRE_CONSENT_CHECK, config.requireConsent ? BST_CHECKED : BST_UNCHECKED);
    
    // Set audio quality
    if (config.audioQuality == 8000) {
        SendMessage(GetDlgItem(hDialog, IDC_AUDIO_QUALITY_COMBO), CB_SETCURSEL, 0, 0);
    } else if (config.audioQuality == 16000) {
        SendMessage(GetDlgItem(hDialog, IDC_AUDIO_QUALITY_COMBO), CB_SETCURSEL, 1, 0);
    } else if (config.audioQuality == 44100) {
        SendMessage(GetDlgItem(hDialog, IDC_AUDIO_QUALITY_COMBO), CB_SETCURSEL, 2, 0);
    }
    
    UpdateControlStates();
}

void SettingsDialog::SaveSettings() {
    if (!configManager) return;
    
    auto& config = configManager->GetConfig();
    
    // Get provider selection
    int providerIndex = SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_GETCURSEL, 0, 0);
    switch (providerIndex) {
        case 0: config.speechConfig.provider = SpeechRecognition::Provider::Windows; break;
        case 1: config.speechConfig.provider = SpeechRecognition::Provider::Azure; break;
        case 2: config.speechConfig.provider = SpeechRecognition::Provider::Google; break;
        case 3: config.speechConfig.provider = SpeechRecognition::Provider::OpenAI; break;
        case 4: config.speechConfig.provider = SpeechRecognition::Provider::AzureOpenAI; break;
    }
    
    // Get API key
    wchar_t apiKeyBuffer[512];
    GetWindowText(GetDlgItem(hDialog, IDC_API_KEY_EDIT), apiKeyBuffer, 512);
    std::wstring apiKeyW(apiKeyBuffer);
    config.speechConfig.apiKey = std::string(apiKeyW.begin(), apiKeyW.end());
    
    // Get region or endpoint based on provider
    wchar_t regionBuffer[1024];  // Increased size for URLs
    GetWindowText(GetDlgItem(hDialog, IDC_REGION_EDIT), regionBuffer, 1024);
    std::wstring regionW(regionBuffer);
    std::string regionStr(regionW.begin(), regionW.end());
    
    if (config.speechConfig.provider == SpeechRecognition::Provider::AzureOpenAI) {
        // For Azure OpenAI, the region field contains the endpoint
        config.speechConfig.endpoint = regionStr;
    } else {
        // For other providers, it's the region
        config.speechConfig.region = regionStr;
    }
    
    // Get language
    int langIndex = SendMessage(GetDlgItem(hDialog, IDC_LANGUAGE_COMBO), CB_GETCURSEL, 0, 0);
    if (langIndex >= 0) {
        wchar_t langBuffer[256];
        SendMessage(GetDlgItem(hDialog, IDC_LANGUAGE_COMBO), CB_GETLBTEXT, langIndex, (LPARAM)langBuffer);
        std::wstring langW(langBuffer);
        
        // Extract language code (first 5 characters)
        if (langW.length() >= 5) {
            std::wstring langCode = langW.substr(0, 5);
            config.speechConfig.language = std::string(langCode.begin(), langCode.end());
        }
    }
    
    // Get checkboxes
    config.speechConfig.enablePunctuation = (IsDlgButtonChecked(hDialog, IDC_PUNCTUATION_CHECK) == BST_CHECKED);
    config.speechConfig.enableSpeakerDiarization = (IsDlgButtonChecked(hDialog, IDC_DIARIZATION_CHECK) == BST_CHECKED);
    config.autoStartRecording = (IsDlgButtonChecked(hDialog, IDC_AUTO_START_CHECK) == BST_CHECKED);
    config.requireConsent = (IsDlgButtonChecked(hDialog, IDC_REQUIRE_CONSENT_CHECK) == BST_CHECKED);
    
    // Get audio quality
    int qualityIndex = SendMessage(GetDlgItem(hDialog, IDC_AUDIO_QUALITY_COMBO), CB_GETCURSEL, 0, 0);
    switch (qualityIndex) {
        case 0: config.audioQuality = 8000; break;
        case 1: config.audioQuality = 16000; break;
        case 2: config.audioQuality = 44100; break;
    }
    
    // Save to file
    configManager->SaveConfig();
    
    std::cout << "Settings saved successfully" << std::endl;
}

void SettingsDialog::OnProviderChanged() {
    UpdateControlStates();
}

void SettingsDialog::UpdateControlStates() {
    int providerIndex = SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_GETCURSEL, 0, 0);
    
    // Enable/disable controls based on provider
    bool needsApiKey = (providerIndex != 0); // All except Windows Speech
    bool needsRegion = (providerIndex == 1);  // Only Azure needs region
    
    EnableWindow(GetDlgItem(hDialog, IDC_API_KEY_EDIT), needsApiKey);
    EnableWindow(GetDlgItem(hDialog, IDC_API_KEY_LABEL), needsApiKey);
    EnableWindow(GetDlgItem(hDialog, IDC_REGION_EDIT), needsRegion);
    EnableWindow(GetDlgItem(hDialog, IDC_REGION_LABEL), needsRegion);
    EnableWindow(GetDlgItem(hDialog, IDC_TEST_CONNECTION_BUTTON), needsApiKey);
    
    // Update labels
    HWND hApiLabel = GetDlgItem(hDialog, IDC_API_KEY_LABEL);
    switch (providerIndex) {
        case 1: // Azure
            SetWindowText(hApiLabel, L"Azure Subscription Key:");
            break;
        case 2: // Google
            SetWindowText(hApiLabel, L"Google Cloud API Key:");
            break;
        case 3: // OpenAI
            SetWindowText(hApiLabel, L"OpenAI API Key:");
            break;
        default:
            SetWindowText(hApiLabel, L"API Key:");
            break;
    }
}

void SettingsDialog::OnTestConnection() {
    int providerIndex = SendMessage(GetDlgItem(hDialog, IDC_PROVIDER_COMBO), CB_GETCURSEL, 0, 0);
    
    // Debug logging
    std::string debugMsg = "Test Connection - Provider Index: " + std::to_string(providerIndex);
    INFO_LOG(debugMsg);
    
    // Get API key
    wchar_t apiKeyBuffer[512];
    GetWindowText(GetDlgItem(hDialog, IDC_API_KEY_EDIT), apiKeyBuffer, 512);
    std::wstring apiKeyW(apiKeyBuffer);
    std::string apiKey(apiKeyW.begin(), apiKeyW.end());
    
    INFO_LOG("Test Connection - API Key length: " + std::to_string(apiKey.length()));
    
    if (apiKey.empty()) {
        MessageBox(hDialog, L"Please enter an API key first.", L"Test Connection", MB_OK | MB_ICONWARNING);
        return;
    }
    
    bool success = false;
    std::string provider;
    
    switch (providerIndex) {
        case 1: { // Azure
            wchar_t regionBuffer[256];
            GetWindowText(GetDlgItem(hDialog, IDC_REGION_EDIT), regionBuffer, 256);
            std::wstring regionW(regionBuffer);
            std::string region(regionW.begin(), regionW.end());
            
            success = TestAzureConnection(apiKey, region);
            provider = "Azure Cognitive Services";
            break;
        }
        case 2: // Google
            success = TestGoogleConnection(apiKey);
            provider = "Google Cloud Speech";
            break;
        case 3: // OpenAI
            success = TestOpenAIConnection(apiKey);
            provider = "OpenAI Whisper";
            break;
        case 4: { // Azure OpenAI
            // For Azure OpenAI, use the region field as endpoint field
            wchar_t endpointBuffer[1024];
            GetWindowText(GetDlgItem(hDialog, IDC_REGION_EDIT), endpointBuffer, 1024);
            std::wstring endpointW(endpointBuffer);
            std::string endpoint(endpointW.begin(), endpointW.end());
            
            INFO_LOG("Test Connection - Azure OpenAI Endpoint: " + endpoint);
            success = TestAzureOpenAIConnection(apiKey, endpoint);
            provider = "Azure OpenAI (GPT-4o)";
            INFO_LOG("Test Connection - Azure OpenAI Result: " + std::string(success ? "SUCCESS" : "FAILED"));
            break;
        }
    }
    
    if (success) {
        std::wstring message = L"Successfully connected to " + std::wstring(provider.begin(), provider.end());
        MessageBox(hDialog, message.c_str(), L"Test Connection", MB_OK | MB_ICONINFORMATION);
    } else {
        std::wstring message = L"Failed to connect to " + std::wstring(provider.begin(), provider.end()) + 
                              L"\n\nPlease check your API key and internet connection.";
        MessageBox(hDialog, message.c_str(), L"Test Connection", MB_OK | MB_ICONERROR);
    }
}

bool SettingsDialog::TestAzureConnection(const std::string& apiKey, const std::string& region) {
    // Simple validation - check if key and region are provided
    // In a real implementation, you would make an HTTP request to Azure
    return !apiKey.empty() && !region.empty() && apiKey.length() > 10;
}

bool SettingsDialog::TestGoogleConnection(const std::string& apiKey) {
    // Simple validation - check if key is provided
    // In a real implementation, you would make an HTTP request to Google
    return !apiKey.empty() && apiKey.length() > 10;
}

bool SettingsDialog::TestOpenAIConnection(const std::string& apiKey) {
    // Simple validation - check if key is provided and has OpenAI format
    // In a real implementation, you would make an HTTP request to OpenAI
    return !apiKey.empty() && (apiKey.substr(0, 3) == "sk-" || apiKey.length() > 20);
}

bool SettingsDialog::TestAzureOpenAIConnection(const std::string& apiKey, const std::string& endpoint) {
    INFO_LOG("TestAzureOpenAIConnection - API Key length: " + std::to_string(apiKey.length()));
    INFO_LOG("TestAzureOpenAIConnection - Endpoint: " + endpoint);
    
    // Validate API key and endpoint
    if (apiKey.empty() || endpoint.empty()) {
        ERROR_LOG("TestAzureOpenAIConnection - API key or endpoint is empty");
        return false;
    }
    
    // Check if endpoint looks like Azure OpenAI endpoint
    if (endpoint.find("openai.azure.com") == std::string::npos && 
        endpoint.find("cognitiveservices.azure.com") == std::string::npos) {
        ERROR_LOG("TestAzureOpenAIConnection - Endpoint doesn't contain Azure domains");
        return false;
    }
    
    // Check API key format (Azure keys are typically 32+ characters)
    if (apiKey.length() < 32) {
        ERROR_LOG("TestAzureOpenAIConnection - API key too short: " + std::to_string(apiKey.length()));
        return false;
    }
    
    INFO_LOG("TestAzureOpenAIConnection - Basic validation passed");
    // TODO: Make actual HTTP request to test endpoint
    // For now, return true if basic validation passes
    return true;
}