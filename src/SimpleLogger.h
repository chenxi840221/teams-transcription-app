#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <windows.h>

// Simple logging macros for debugging
class SimpleLogger {
private:
    static std::ofstream logFile;
    static bool initialized;
    
public:
    static void Initialize() {
        if (!initialized) {
            logFile.open("debug.log", std::ios::app);
            initialized = true;
            LogMessage("INFO", "=== LOGGER INITIALIZED ===");
        }
    }
    
    static void LogMessage(const std::string& level, const std::string& message) {
        time_t now = time(0);
        tm* timeinfo = localtime(&now);
        char timestamp[100];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        std::string logEntry = std::string(timestamp) + " [" + level + "] " + message;
        
        // Output to console
        std::cout << logEntry << std::endl;
        
        // Output to file
        if (logFile.is_open()) {
            logFile << logEntry << std::endl;
            logFile.flush();
        }
    }
    
    static void Close() {
        if (logFile.is_open()) {
            LogMessage("INFO", "=== LOGGER SHUTTING DOWN ===");
            logFile.close();
        }
    }
};

// Static member initialization - moved to cpp file to avoid multiple definitions

// Convenience macros
#define DEBUG_LOG(msg) SimpleLogger::LogMessage("DEBUG", msg)
#define INFO_LOG(msg) SimpleLogger::LogMessage("INFO", msg)
#define WARN_LOG(msg) SimpleLogger::LogMessage("WARN", msg)
#define ERROR_LOG(msg) SimpleLogger::LogMessage("ERROR", msg)
#define AUDIO_LOG(stage, size, format) SimpleLogger::LogMessage("AUDIO", std::string("[") + stage + "] Size: " + std::to_string(size) + " bytes, " + format)
#define CONFIG_LOG(key, value) SimpleLogger::LogMessage("CONFIG", std::string("[") + key + "] = " + value)