@echo off
echo Setting up Teams Audio Transcription Application...

echo Checking prerequisites...

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo CMake not found! Please install CMake 3.20 or later.
    pause
    exit /b 1
)

where cl >nul 2>nul
if %errorlevel% neq 0 (
    echo Visual Studio compiler not found! Please install Visual Studio 2019 or later.
    pause
    exit /b 1
)

echo Creating output directories...
if not exist data\recordings mkdir data\recordings
if not exist data\transcripts mkdir data\transcripts
if not exist data\exports mkdir data\exports

echo Copying default configuration...
if not exist config\settings.json (
    echo Default configuration already exists.
)

echo Setup completed!
echo Next steps:
echo 1. Configure your API keys in config\settings.json
echo 2. Run scripts\build.bat to build the application
echo 3. Run the executable from build\output\Release\
pause
