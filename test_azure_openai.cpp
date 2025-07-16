#include <iostream>
#include <string>
#include <windows.h>
#include <winhttp.h>
#include <vector>

#pragma comment(lib, "winhttp.lib")

// Test utility for Azure OpenAI Speech Recognition
class AzureOpenAITester {
private:
    std::string apiKey;
    std::string endpoint;
    
public:
    AzureOpenAITester(const std::string& key, const std::string& ep) 
        : apiKey(key), endpoint(ep) {}
    
    bool TestConnection() {
        std::wcout << L"Testing Azure OpenAI connection..." << std::endl;
        std::wcout << L"Endpoint: " << std::wstring(endpoint.begin(), endpoint.end()) << std::endl;
        std::wcout << L"API Key: " << (apiKey.empty() ? L"EMPTY" : L"SET") << std::endl;
        
        // Basic validation
        if (apiKey.empty() || endpoint.empty()) {
            std::wcout << L"ERROR: API key or endpoint is empty" << std::endl;
            return false;
        }
        
        // Check endpoint format
        if (endpoint.find("cognitiveservices.azure.com") == std::string::npos) {
            std::wcout << L"ERROR: Endpoint doesn't look like Azure OpenAI" << std::endl;
            return false;
        }
        
        // Check API key format (Azure keys are typically 32+ chars)
        if (apiKey.length() < 32) {
            std::wcout << L"ERROR: API key too short for Azure" << std::endl;
            return false;
        }
        
        std::wcout << L"✅ Basic validation passed" << std::endl;
        
        // Parse URL
        std::wstring wEndpoint(endpoint.begin(), endpoint.end());
        URL_COMPONENTS urlComp = {};
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwSchemeLength = -1;
        urlComp.dwHostNameLength = -1;
        urlComp.dwUrlPathLength = -1;
        urlComp.dwExtraInfoLength = -1;
        
        if (!WinHttpCrackUrl(wEndpoint.c_str(), 0, 0, &urlComp)) {
            std::wcout << L"ERROR: Invalid URL format" << std::endl;
            return false;
        }
        
        std::wstring hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);
        std::wstring path(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
        std::wstring query = urlComp.lpszExtraInfo ? std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength) : L"";
        
        std::wcout << L"Host: " << hostName << std::endl;
        std::wcout << L"Path: " << path << query << std::endl;
        
        // Test basic HTTP connection
        return TestHttpConnection(hostName, path + query);
    }
    
private:
    bool TestHttpConnection(const std::wstring& hostName, const std::wstring& path) {
        std::wcout << L"Testing HTTP connection..." << std::endl;
        
        // Initialize WinHTTP
        HINTERNET hSession = WinHttpOpen(L"Azure OpenAI Tester/1.0",
            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS, 0);
            
        if (!hSession) {
            std::wcout << L"ERROR: Failed to initialize WinHTTP" << std::endl;
            return false;
        }
        
        // Connect to server
        HINTERNET hConnect = WinHttpConnect(hSession, hostName.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT, 0);
            
        if (!hConnect) {
            std::wcout << L"ERROR: Failed to connect to server" << std::endl;
            WinHttpCloseHandle(hSession);
            return false;
        }
        
        // Create request (HEAD to test connectivity without sending data)
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", path.c_str(),
            NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_SECURE);
            
        if (!hRequest) {
            std::wcout << L"ERROR: Failed to create request" << std::endl;
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }
        
        // Add API key header
        std::wstring apiKeyHeader = L"api-key: " + std::wstring(apiKey.begin(), apiKey.end());
        WinHttpAddRequestHeaders(hRequest, apiKeyHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
        
        // Send request
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            std::wcout << L"ERROR: Failed to send request" << std::endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }
        
        // Receive response
        if (!WinHttpReceiveResponse(hRequest, NULL)) {
            std::wcout << L"ERROR: Failed to receive response" << std::endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return false;
        }
        
        // Get status code
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);
            
        std::wcout << L"HTTP Status: " << statusCode << std::endl;
        
        // Clean up
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        
        // Check status codes
        if (statusCode == 200 || statusCode == 405) { // 405 Method Not Allowed is expected for HEAD requests
            std::wcout << L"✅ Connection successful!" << std::endl;
            return true;
        } else if (statusCode == 401) {
            std::wcout << L"❌ Authentication failed (401) - Check API key" << std::endl;
            return false;
        } else if (statusCode == 404) {
            std::wcout << L"❌ Endpoint not found (404) - Check URL" << std::endl;
            return false;
        } else {
            std::wcout << L"❌ Unexpected status code: " << statusCode << std::endl;
            return false;
        }
    }
};

int main() {
    std::wcout << L"=== Azure OpenAI Connection Tester ===" << std::endl;
    
    // Test with your credentials
    std::string apiKey = "DD72UKb3M6qFvh0ay7Vzr3HDQaLQv2yJfKoEeYDJZwSEC1ODoQcSJQQJ99BGACHYHv6XJ3w3AAAAACOGcrjb";
    std::string endpoint = "https://xich-md2jxcth-eastus2.cognitiveservices.azure.com/openai/deployments/gpt-4o-transcribe/audio/transcriptions?api-version=2025-03-01-preview";
    
    AzureOpenAITester tester(apiKey, endpoint);
    bool success = tester.TestConnection();
    
    std::wcout << std::endl;
    std::wcout << L"Result: " << (success ? L"✅ SUCCESS" : L"❌ FAILED") << std::endl;
    
    std::wcout << L"Press Enter to continue...";
    std::cin.get();
    
    return success ? 0 : 1;
}