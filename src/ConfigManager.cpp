#include "ConfigManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

using json = nlohmann::json;

ConfigManager::ConfigManager() {
    SetDefaultConfig();
    configFilePath = "config/settings.json";
}

ConfigManager::~ConfigManager() = default;

bool ConfigManager::LoadConfig(const std::string& configPath) {
    std::string path = configPath.empty() ? configFilePath : configPath;
    
    try {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Config file not found: " << path << std::endl;
            return false;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << path << std::endl;
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        return ParseJsonConfig(content);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::SaveConfig(const std::string& configPath) {
    std::string path = configPath.empty() ? configFilePath : configPath;
    
    try {
        // Ensure directory exists
        std::filesystem::path filePath(path);
        std::filesystem::create_directories(filePath.parent_path());

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to create config file: " << path << std::endl;
            return false;
        }

        std::string jsonContent = GenerateJsonConfig();
        file << jsonContent;
        file.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::SetSpeechConfig(const SpeechRecognition::SpeechConfig& speechConfig) {
    config.speechConfig = speechConfig;
}

void ConfigManager::SetDefaultConfig() {
    // Recording settings
    config.autoStartRecording = false;
    config.outputFormat = "wav";
    config.outputDirectory = "./data/recordings";
    config.audioQuality = 16000;

    // Speech recognition
    config.speechConfig.provider = SpeechRecognition::Provider::Azure;
    config.speechConfig.apiKey = "";
    config.speechConfig.region = "eastus";
    config.speechConfig.language = "en-US";
    config.speechConfig.enablePunctuation = true;
    config.speechConfig.enableSpeakerDiarization = true;

    // UI settings
    config.minimizeToTray = true;
    config.showNotifications = true;
    config.theme = "system";

    // Privacy settings
    config.requireConsent = true;
    config.dataRetentionDays = 30;
    config.enableEncryption = true;

    // Export settings
    config.exportFormats = {"txt", "docx", "pdf"};
    config.autoExport = false;
    config.exportTemplate = "meeting-transcript";
}

bool ConfigManager::ParseJsonConfig(const std::string& jsonContent) {
    try {
        json j = json::parse(jsonContent);

        // Recording settings
        if (j.contains("recording")) {
            auto& recording = j["recording"];
            if (recording.contains("autoStart")) {
                config.autoStartRecording = recording["autoStart"].get<bool>();
            }
            if (recording.contains("outputFormat")) {
                config.outputFormat = recording["outputFormat"].get<std::string>();
            }
            if (recording.contains("outputDirectory")) {
                config.outputDirectory = recording["outputDirectory"].get<std::string>();
            }
            if (recording.contains("audioQuality")) {
                config.audioQuality = recording["audioQuality"].get<int>();
            }
        }

        // Speech recognition settings
        if (j.contains("speechRecognition")) {
            auto& speech = j["speechRecognition"];
            if (speech.contains("provider")) {
                std::string provider = speech["provider"].get<std::string>();
                if (provider == "azure") {
                    config.speechConfig.provider = SpeechRecognition::Provider::Azure;
                } else if (provider == "google") {
                    config.speechConfig.provider = SpeechRecognition::Provider::Google;
                } else if (provider == "openai") {
                    config.speechConfig.provider = SpeechRecognition::Provider::OpenAI;
                } else if (provider == "amazon") {
                    config.speechConfig.provider = SpeechRecognition::Provider::Amazon;
                } else if (provider == "windows") {
                    config.speechConfig.provider = SpeechRecognition::Provider::Windows;
                }
            }
            if (speech.contains("apiKey")) {
                config.speechConfig.apiKey = speech["apiKey"].get<std::string>();
            }
            if (speech.contains("region")) {
                config.speechConfig.region = speech["region"].get<std::string>();
            }
            if (speech.contains("language")) {
                config.speechConfig.language = speech["language"].get<std::string>();
            }
            if (speech.contains("enablePunctuation")) {
                config.speechConfig.enablePunctuation = speech["enablePunctuation"].get<bool>();
            }
            if (speech.contains("enableSpeakerDiarization")) {
                config.speechConfig.enableSpeakerDiarization = speech["enableSpeakerDiarization"].get<bool>();
            }
        }

        // UI settings
        if (j.contains("ui")) {
            auto& ui = j["ui"];
            if (ui.contains("minimizeToTray")) {
                config.minimizeToTray = ui["minimizeToTray"].get<bool>();
            }
            if (ui.contains("showNotifications")) {
                config.showNotifications = ui["showNotifications"].get<bool>();
            }
            if (ui.contains("theme")) {
                config.theme = ui["theme"].get<std::string>();
            }
        }

        // Privacy settings
        if (j.contains("privacy")) {
            auto& privacy = j["privacy"];
            if (privacy.contains("requireConsent")) {
                config.requireConsent = privacy["requireConsent"].get<bool>();
            }
            if (privacy.contains("dataRetentionDays")) {
                config.dataRetentionDays = privacy["dataRetentionDays"].get<int>();
            }
            if (privacy.contains("enableEncryption")) {
                config.enableEncryption = privacy["enableEncryption"].get<bool>();
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing JSON config: " << e.what() << std::endl;
        return false;
    }
}

std::string ConfigManager::GenerateJsonConfig() const {
    json j;

    // Recording settings
    j["recording"]["autoStart"] = config.autoStartRecording;
    j["recording"]["outputFormat"] = config.outputFormat;
    j["recording"]["outputDirectory"] = config.outputDirectory;
    j["recording"]["audioQuality"] = config.audioQuality;

    // Speech recognition settings
    std::string providerStr = "azure";
    switch (config.speechConfig.provider) {
        case SpeechRecognition::Provider::Azure: providerStr = "azure"; break;
        case SpeechRecognition::Provider::Google: providerStr = "google"; break;
        case SpeechRecognition::Provider::OpenAI: providerStr = "openai"; break;
        case SpeechRecognition::Provider::Amazon: providerStr = "amazon"; break;
        case SpeechRecognition::Provider::Windows: providerStr = "windows"; break;
    }
    
    j["speechRecognition"]["provider"] = providerStr;
    j["speechRecognition"]["apiKey"] = config.speechConfig.apiKey;
    j["speechRecognition"]["region"] = config.speechConfig.region;
    j["speechRecognition"]["language"] = config.speechConfig.language;
    j["speechRecognition"]["enablePunctuation"] = config.speechConfig.enablePunctuation;
    j["speechRecognition"]["enableSpeakerDiarization"] = config.speechConfig.enableSpeakerDiarization;

    // UI settings
    j["ui"]["minimizeToTray"] = config.minimizeToTray;
    j["ui"]["showNotifications"] = config.showNotifications;
    j["ui"]["theme"] = config.theme;

    // Privacy settings
    j["privacy"]["requireConsent"] = config.requireConsent;
    j["privacy"]["dataRetentionDays"] = config.dataRetentionDays;
    j["privacy"]["enableEncryption"] = config.enableEncryption;

    return j.dump(4);  // Pretty print with 4-space indentation
}