# Test Settings Dialog - Azure OpenAI Connection

## Steps to Test

1. **Run the Application**
   ```bash
   cd build/output/Debug
   ./TeamsTranscriptionApp.exe
   ```

2. **Open Settings Dialog**
   - Click the "Settings" button in the main window
   - OR use menu option if available

3. **Configure Azure OpenAI Settings**
   - **Provider Dropdown**: Select "Azure OpenAI (GPT-4o)" (should be option 5)
   - **API Key Field**: Enter your API key:
     ```
     DD72UKb3M6qFvh0ay7Vzr3HDQaLQv2yJfKoEeYDJZwSEC1ODoQcSJQQJ99BGACHYHv6XJ3w3AAAAACOGcrjb
     ```
   - **Region/Endpoint Field**: Enter your endpoint URL:
     ```
     https://xich-md2jxcth-eastus2.cognitiveservices.azure.com/openai/deployments/gpt-4o-transcribe/audio/transcriptions?api-version=2025-03-01-preview
     ```

4. **Test Connection**
   - Click "Test Connection" button
   - Check the result message

5. **Check Debug Logs**
   - Open another terminal/command prompt
   - Navigate to: `build/output/Debug/`
   - Monitor logs: `tail -f debug.log`
   - Look for log entries starting with "Test Connection"

## Expected Behavior

### If Working Correctly:
- Success message: "Successfully connected to Azure OpenAI (GPT-4o)"
- Debug logs should show:
  ```
  [INFO] Test Connection - Provider Index: 4
  [INFO] Test Connection - API Key length: 84
  [INFO] Test Connection - Azure OpenAI Endpoint: https://...
  [INFO] TestAzureOpenAIConnection - API Key length: 84
  [INFO] TestAzureOpenAIConnection - Endpoint: https://...
  [INFO] TestAzureOpenAIConnection - Basic validation passed
  [INFO] Test Connection - Azure OpenAI Result: SUCCESS
  ```

### If Still Failing:
- Error message: "Failed to connect to Azure OpenAI (GPT-4o)"
- Debug logs will show which validation failed:
  - `ERROR_LOG("TestAzureOpenAIConnection - API key or endpoint is empty")`
  - `ERROR_LOG("TestAzureOpenAIConnection - Endpoint doesn't contain Azure domains")`
  - `ERROR_LOG("TestAzureOpenAIConnection - API key too short")`

## Troubleshooting

### Common Issues:

1. **Wrong Provider Selected**
   - Make sure "Azure OpenAI (GPT-4o)" is selected (index 4)
   - Check debug log for "Provider Index: 4"

2. **Empty Fields**
   - Ensure API key field is filled
   - Ensure endpoint field is filled
   - Check debug log for field lengths

3. **Wrong Field Mapping**
   - The Region field is being used for the endpoint
   - Make sure endpoint URL is in the "Region" field, not a separate endpoint field

4. **Provider Loading Issue**
   - The settings might not be loading the current configuration properly
   - Try manually entering the values even if they appear filled

## Fixed Issues

- ✅ Added case 4 for Azure OpenAI in Test Connection switch
- ✅ Added TestAzureOpenAIConnection method
- ✅ Fixed LoadSettings to load endpoint into Region field for Azure OpenAI  
- ✅ Fixed SaveSettings to save endpoint from Region field for Azure OpenAI
- ✅ Added comprehensive debug logging
- ✅ Added SimpleLogger.h include to SettingsDialog.cpp

## Next Steps

If the test still fails, the debug logs will show exactly which validation is failing. The current implementation does basic validation only:

1. Checks if API key and endpoint are not empty
2. Validates endpoint contains Azure domain  
3. Checks API key length (>= 32 characters)

If basic validation passes, it should show SUCCESS. If it still shows FAILED, check the debug logs for the specific error message.