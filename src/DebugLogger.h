#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <windows.h>

class DebugLogger {
public:
    enum class Level {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3
    };

    static DebugLogger& Instance() {
        static DebugLogger instance;
        return instance;
    }

    void Initialize(const std::string& logFilePath = "debug.log") {
        this->logFilePath = logFilePath;
        
        // Open log file
        logFile.open(logFilePath, std::ios::app);
        if (logFile.is_open()) {
            Log(Level::INFO, "=== DEBUG LOGGER INITIALIZED ===");
        }
    }

    void Log(Level level, const std::string& message) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();
        ss << " [" << LevelToString(level) << "] " << message;

        std::string logEntry = ss.str();

        // Output to console
        std::cout << logEntry << std::endl;

        // Output to file
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.flush();
        }
    }

    void LogAudioData(const std::string& stage, size_t dataSize, const std::string& format = "") {
        std::stringstream ss;
        ss << "AUDIO [" << stage << "] Size: " << dataSize << " bytes";
        if (!format.empty()) {
            ss << ", Format: " << format;
        }
        Log(Level::DEBUG, ss.str());
    }

    void LogConfig(const std::string& key, const std::string& value) {
        Log(Level::INFO, "CONFIG [" + key + "] = " + value);
    }

    template<typename T>
    void SaveAudioSample(const std::vector<T>& audioData, const std::string& filename) {
        try {
            // Create debug_audio directory if it doesn't exist
            CreateDirectoryA("debug_audio", NULL);
            
            std::string fullPath = "debug_audio/" + filename;
            
            std::ofstream file(fullPath, std::ios::binary);
            if (file.is_open()) {
                file.write(reinterpret_cast<const char*>(audioData.data()), audioData.size() * sizeof(T));
                file.close();
                Log(Level::INFO, "Audio sample saved: " + fullPath + " (" + std::to_string(audioData.size() * sizeof(T)) + " bytes)");
            } else {
                Log(Level::ERROR, "Failed to save audio sample: " + fullPath);
            }
        } catch (const std::exception& e) {
            Log(Level::ERROR, "Error saving audio sample: " + std::string(e.what()));
        }
    }

    ~DebugLogger() {
        if (logFile.is_open()) {
            Log(Level::INFO, "=== DEBUG LOGGER SHUTTING DOWN ===");
            logFile.close();
        }
    }

private:
    DebugLogger() = default;
    std::ofstream logFile;
    std::string logFilePath;

    std::string LevelToString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARN";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Convenience macros
#define DEBUG_LOG(message) DebugLogger::Instance().Log(DebugLogger::Level::DEBUG, message)
#define INFO_LOG(message) DebugLogger::Instance().Log(DebugLogger::Level::INFO, message)
#define WARN_LOG(message) DebugLogger::Instance().Log(DebugLogger::Level::WARNING, message)
#define ERROR_LOG(message) DebugLogger::Instance().Log(DebugLogger::Level::ERROR, message)
#define AUDIO_LOG(stage, size, format) DebugLogger::Instance().LogAudioData(stage, size, format)
#define CONFIG_LOG(key, value) DebugLogger::Instance().LogConfig(key, value)