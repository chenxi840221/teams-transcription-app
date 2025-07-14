# Teams Audio Transcription Application

A Windows desktop application for capturing and transcribing Teams meeting audio using low-level Windows APIs.

## Features

- Real-time audio capture from system output
- Multiple speech recognition providers (Azure, Google, OpenAI, etc.)
- Teams process monitoring and meeting detection
- Modern Windows GUI with system tray integration
- Export capabilities (TXT, DOCX, PDF)
- Configurable privacy and consent settings

## Quick Start

1. Run `scripts\setup.bat` to initialize the project
2. Configure API keys in `config\settings.json`
3. Run `scripts\build.bat` to compile
4. Execute `build\output\Release\TeamsTranscriptionApp.exe`

## Requirements

- Windows 10 version 1903 or later
- Visual Studio 2019/2022 with C++ development tools
- CMake 3.20 or higher
- Speech recognition API keys (Azure, Google, etc.)

## Configuration

Edit `config\settings.json` to configure:
- Speech recognition provider and API keys
- Audio quality and output settings
- Privacy and consent requirements
- Export formats and templates

## Legal Notice

âš ï¸ **Important**: Always obtain explicit consent from all meeting participants before recording. Ensure compliance with local laws and organizational policies.

## Support

For issues and questions, please check the documentation in the `docs` folder.
