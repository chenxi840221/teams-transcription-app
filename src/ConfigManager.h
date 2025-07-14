#pragma once

#include "SpeechRecognition.h"
#include <string>
#include <memory>

class ConfigManager {
public:
    struct AppConfig {
        // Recording settings
        bool autoStartRecording;
        std::string outputFormat;
        std::string outputDirectory;
        int audioQuality;
        
        // Speech recognition
        SpeechRecognition::SpeechConfig speechConfig;
        
        // UI settings
        bool minimizeToTray;
        bool showNotifications;
        std::string theme;
        
        // Privacy settings
        bool requireConsent;
        int dataRetentionDays;
        bool enableEncryption;
        
        // Export settings
        std::vector<std::string> exportFormats;
        bool autoExport;
        std::string exportTemplate;
    };

    ConfigManager();
    ~ConfigManager();

    bool LoadConfig(const std::string& configPath = "");
    bool SaveConfig(const std::string& configPath = "");
    
    const AppConfig& GetConfig() const { return config; }
    AppConfig& GetConfig() { return config; }
    
    SpeechRecognition::SpeechConfig GetSpeechConfig() const { return config.speechConfig; }
    void SetSpeechConfig(const SpeechRecognition::SpeechConfig& speechConfig);

private:
    AppConfig config;
    std::string configFilePath;

    void SetDefaultConfig();
    bool ParseJsonConfig(const std::string& jsonContent);
    std::string GenerateJsonConfig() const;
};
