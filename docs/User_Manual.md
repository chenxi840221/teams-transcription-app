# Teams Audio Transcription Application
## User Manual

**Version:** 2.0.0  
**Date:** July 2025  
**Document Type:** End-User Documentation  

---

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [Basic Operations](#2-basic-operations)
3. [Configuration](#3-configuration)
4. [Advanced Features](#4-advanced-features)
5. [Troubleshooting](#5-troubleshooting)
6. [Legal and Privacy](#6-legal-and-privacy)
7. [FAQ](#7-faq)
8. [Support](#8-support)

---

## 1. Getting Started

### 1.1 Welcome to Teams Audio Transcription

The Teams Audio Transcription Application is a powerful tool that helps you automatically transcribe Microsoft Teams meeting audio in real-time. This application captures system audio and converts speech to text using advanced speech recognition technology.

### 1.2 What You Can Do

✅ **Real-time Transcription**: Convert Teams meeting audio to text as it happens  
✅ **Multiple Speech Providers**: Choose from Azure, Google, OpenAI, and Windows speech recognition  
✅ **Export Options**: Save transcripts in various formats (TXT, DOCX, PDF)  
✅ **Meeting Detection**: Automatically detect when you're in a Teams meeting  
✅ **Privacy Controls**: All processing happens locally with configurable privacy settings  

### 1.3 System Requirements

**Minimum Requirements:**
- Windows 10 version 1903 or later
- 4GB RAM
- 1GB available disk space
- Active internet connection for cloud speech recognition
- Audio device (speakers/headphones)

**Recommended:**
- Windows 11
- 8GB RAM
- 10GB available disk space
- High-quality headset for better audio capture

### 1.4 First Launch

When you first start the application:

1. **Accept License Agreement**: Review and accept the terms of use
2. **Privacy Consent**: Understand the recording implications and legal requirements
3. **Initial Configuration**: Set up your preferred speech recognition provider
4. **Audio Test**: Verify that audio capture is working correctly

---

## 2. Basic Operations

### 2.1 Main Interface Overview

```
┌─────────────────────────────────────────────────────────────────┐
│  [Start] [Stop] [Pause] [Settings] [Export] [Clear]           │
├─────────────────────────────────────────────────────────────────┤
│  Teams Status: In Meeting - Weekly Standup    [●●●●●○○○○○]     │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  [10:30:15] Thank you for joining today's meeting.     │   │
│  │  [10:30:45] Let me share my screen with everyone.      │   │
│  │  [10:31:12] I can see the presentation clearly now.    │   │
│  │                                                         │   │
│  │             Live Transcription Area                     │   │
│  │                                                         │   │
│  └─────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│  Status: Recording... | Frames: 1,245,680 | Time: 00:15:32     │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 Starting a Recording Session

#### Method 1: Manual Start
1. Open the Teams Transcription Application
2. Join your Teams meeting first
3. Click the **Start** button in the application
4. Confirm the recording consent dialog
5. The application will begin capturing and transcribing audio

#### Method 2: Auto-Start (if configured)
1. Configure auto-start in Settings → Recording → Auto Start
2. When you join a Teams meeting, recording will start automatically
3. You'll see a notification in the system tray

### 2.3 During a Recording Session

**What You'll See:**
- **Live Transcription**: Text appears in real-time as people speak
- **Timestamps**: Each segment shows when it was spoken
- **Teams Status**: Shows your current meeting status
- **Audio Levels**: Visual indication of audio capture activity
- **Progress Bar**: Shows recording progress

**What You Can Do:**
- **Pause/Resume**: Click Pause to temporarily stop transcription
- **Monitor**: Watch the live transcription for accuracy
- **Scroll**: Review earlier parts of the conversation
- **Continue**: Let the application run in the background

### 2.4 Stopping a Recording Session

1. Click the **Stop** button
2. The application will:
   - Stop audio capture
   - Finalize the transcript
   - Auto-save to your configured location
   - Show completion notification

### 2.5 System Tray Operation

The application can minimize to the system tray for unobtrusive operation:

- **Tray Icon**: Shows application status (recording/stopped)
- **Double-click**: Restores the main window
- **Right-click**: Shows context menu with quick actions
- **Notifications**: Alerts you to important events

---

## 3. Configuration

### 3.1 Accessing Settings

Click the **Settings** button in the main window to open the configuration dialog.

### 3.2 Recording Settings

#### Audio Quality
- **Low (8kHz)**: Suitable for voice-only, minimal bandwidth
- **Medium (16kHz)**: Recommended for most meetings ⭐
- **High (44kHz)**: Best quality, requires more resources

#### Output Format
- **WAV**: Uncompressed audio (if audio saving is enabled)
- **MP3**: Compressed audio (smaller file size)

#### Storage Location
- Choose where recordings and transcripts are saved
- Default: `Documents\Teams Transcriptions`
- Ensure sufficient disk space

### 3.3 Speech Recognition Settings

#### Provider Selection
Choose your preferred speech recognition service:

**Azure Cognitive Services** ⭐ *Recommended*
- **Pros**: High accuracy, speaker diarization, multiple languages
- **Cons**: Requires API key and internet connection
- **Setup**: Enter your Azure subscription key and region

**Google Cloud Speech-to-Text**
- **Pros**: Excellent accuracy, good language support
- **Cons**: Requires API key and billing account
- **Setup**: Enter your Google Cloud API key

**OpenAI Whisper**
- **Pros**: Very high accuracy, good for various accents
- **Cons**: Higher cost, requires API key
- **Setup**: Enter your OpenAI API key

**Windows Speech Recognition**
- **Pros**: No API key required, works offline
- **Cons**: Lower accuracy, limited features
- **Setup**: Automatic, no configuration needed

#### Language Settings
- **Primary Language**: Select your meeting language
- **Enable Punctuation**: Automatically add periods, commas, etc.
- **Speaker Diarization**: Identify different speakers (where supported)

### 3.4 Privacy Settings

#### Consent Management
- **Require Consent**: Always ask before starting recording ⭐ *Recommended*
- **Consent Timeout**: How long to wait for consent response
- **Participant Notification**: Display recording indicator

#### Data Retention
- **Keep Transcripts**: How long to store transcripts (7-365 days)
- **Auto-Delete**: Automatically remove old transcripts
- **Encryption**: Encrypt stored files (recommended)

#### Privacy Mode
- **Local Only**: Never send data to cloud services
- **Minimal Cloud**: Only use cloud for speech recognition
- **Enhanced**: Allow cloud features and analytics

### 3.5 User Interface Settings

#### Display Options
- **Theme**: System, Light, or Dark mode
- **Font Size**: Small, Medium, Large, Extra Large
- **Show Timestamps**: Display time for each transcript segment
- **Show Confidence**: Display recognition confidence scores

#### Behavior
- **Minimize to Tray**: Hide window when minimized
- **Start with Windows**: Launch automatically at startup
- **Show Notifications**: Display system notifications
- **Auto-scroll**: Keep latest text visible

### 3.6 Export Settings

#### Default Formats
Select which formats to generate automatically:
- ☑ Plain Text (.txt)
- ☑ Microsoft Word (.docx)
- ☐ PDF Document (.pdf)
- ☐ JSON Data (.json)

#### Export Templates
- **Meeting Minutes**: Formatted for formal meetings
- **Interview**: Optimized for interviews and conversations
- **Custom**: Define your own template

---

## 4. Advanced Features

### 4.1 Export and Sharing

#### Export Options
After a recording session, you can export transcripts in multiple formats:

**Plain Text Export**
- Clean, readable text format
- Includes timestamps and speaker labels
- Easy to copy and paste

**Microsoft Word Export**
- Professional formatting
- Header with meeting details
- Speaker names highlighted
- Customizable templates

**PDF Export**
- Print-ready format
- Consistent formatting across devices
- Password protection option

#### Export Process
1. Click **Export** in the main window
2. Choose your desired format(s)
3. Select export template
4. Choose save location
5. Click **Generate**

### 4.2 Search and Navigation

#### Search Functionality
- **Text Search**: Find specific words or phrases in transcripts
- **Speaker Search**: Find all text from a specific speaker
- **Time Range**: Search within specific time periods
- **Regular Expressions**: Advanced pattern matching

#### Navigation Features
- **Jump to Time**: Click timestamps to hear audio at that point
- **Speaker View**: Filter to show only specific speakers
- **Bookmark**: Mark important sections for quick access

### 4.3 Integration Features

#### Calendar Integration
- **Meeting Detection**: Automatically detect scheduled meetings
- **Auto-naming**: Use calendar titles for transcript names
- **Participant List**: Import participant information

#### File Management
- **Automatic Organization**: Group transcripts by date/meeting
- **Backup System**: Automatic backup to secondary location
- **Cloud Sync**: Optional sync with cloud storage services

### 4.4 Keyboard Shortcuts

| Action | Shortcut | Description |
|--------|----------|-------------|
| Start Recording | `Ctrl + R` | Begin audio capture |
| Stop Recording | `Ctrl + S` | End current session |
| Pause/Resume | `Ctrl + P` | Toggle pause state |
| Export | `Ctrl + E` | Open export dialog |
| Settings | `Ctrl + ,` | Open settings window |
| Clear Transcript | `Ctrl + Del` | Clear current text |
| Search | `Ctrl + F` | Open search dialog |
| New Session | `Ctrl + N` | Start new transcript |

---

## 5. Troubleshooting

### 5.1 Common Issues

#### No Audio Captured
**Symptoms**: Recording starts but no text appears
**Solutions**:
1. Check that Teams audio is playing through speakers/headphones
2. Verify audio device selection in Windows Sound settings
3. Ensure application has microphone permissions
4. Test with a different audio device
5. Restart the application and try again

#### Poor Transcription Accuracy
**Symptoms**: Text is incorrect or garbled
**Solutions**:
1. Improve audio quality by using a headset
2. Reduce background noise in your environment
3. Switch to a different speech recognition provider
4. Check internet connection for cloud providers
5. Speak more clearly and at moderate pace

#### Application Won't Start
**Symptoms**: Application fails to launch or crashes immediately
**Solutions**:
1. Run as Administrator
2. Check Windows compatibility mode
3. Update Visual C++ Redistributables
4. Verify system requirements are met
5. Check Windows Event Viewer for error details

#### Teams Not Detected
**Symptoms**: Shows "Teams not detected" even when Teams is running
**Solutions**:
1. Ensure Teams desktop app is running (not web version)
2. Restart both Teams and the transcription application
3. Check if Teams is running as Administrator
4. Verify Teams window titles are in English
5. Update to latest Teams version

### 5.2 Performance Issues

#### High CPU Usage
**Symptoms**: Computer runs slowly during transcription
**Solutions**:
1. Lower audio quality setting (use Medium instead of High)
2. Close unnecessary applications
3. Use Windows Speech Recognition instead of cloud providers
4. Increase virtual memory/page file size
5. Consider hardware upgrade if issues persist

#### Memory Usage Growing
**Symptoms**: Application uses increasing amounts of RAM
**Solutions**:
1. Enable auto-save with shorter intervals
2. Clear transcript regularly for long meetings
3. Restart application for very long sessions (4+ hours)
4. Check for memory leaks in Event Viewer
5. Update to latest application version

### 5.3 API and Connection Issues

#### Azure Speech Service Errors
**Error Messages**: "Authentication failed", "Quota exceeded"
**Solutions**:
1. Verify API key is correct and active
2. Check Azure subscription status and billing
3. Ensure selected region matches your subscription
4. Monitor API usage against quotas
5. Contact Azure support if issues persist

#### Network Connection Problems
**Symptoms**: Intermittent transcription or connection errors
**Solutions**:
1. Check internet connection stability
2. Configure firewall exceptions for the application
3. Try different DNS servers (8.8.8.8, 1.1.1.1)
4. Use VPN if behind corporate firewall
5. Switch to offline Windows Speech Recognition

### 5.4 File and Data Issues

#### Cannot Save Transcripts
**Error Messages**: "Access denied", "Disk full"
**Solutions**:
1. Check available disk space (need at least 1GB free)
2. Verify write permissions to save directory
3. Choose different save location
4. Run application as Administrator
5. Check antivirus isn't blocking file operations

#### Corrupted Transcript Files
**Symptoms**: Cannot open saved transcripts
**Solutions**:
1. Check backup copies in backup folder
2. Use file recovery tools for critical data
3. Export to different format as backup
4. Enable file encryption for protection
5. Regular backup to external location

---

## 6. Legal and Privacy

### 6.1 Recording Consent Requirements

⚠️ **Important Legal Notice**

Recording conversations may be subject to local laws and regulations. You are responsible for ensuring compliance with applicable laws in your jurisdiction.

#### United States
- **One-party consent states**: You may record if you're a participant
- **Two-party consent states**: All participants must consent
- **Federal law**: Generally allows recording with one-party consent

#### International Considerations
- **European Union**: GDPR requires explicit consent
- **Canada**: Similar to US with provincial variations
- **Australia**: One-party consent generally allowed
- **United Kingdom**: Complex rules, legal advice recommended

#### Best Practices
1. **Always inform participants** that recording is taking place
2. **Obtain explicit consent** from all participants when possible
3. **Document consent** for your records
4. **Respect participant wishes** if they decline
5. **Consult legal counsel** for business use

### 6.2 Data Privacy and Security

#### Local Data Processing
- All audio processing happens on your local computer
- No audio data is transmitted unless using cloud speech services
- Transcripts are stored locally by default
- You control all data retention and deletion

#### Cloud Service Considerations
When using cloud speech recognition services:
- Audio data is temporarily sent to speech provider
- Data is processed according to provider's privacy policy
- Most providers delete data after processing
- Consider privacy policies of Azure, Google, OpenAI

#### Data Protection Measures
- **Encryption**: All stored files can be encrypted
- **Access Control**: Only you have access to transcripts
- **Secure Deletion**: Files are securely overwritten when deleted
- **No Analytics**: No usage data is collected or transmitted

### 6.3 Microsoft Teams Compliance

#### Terms of Service
- This application does not violate Teams Terms of Service
- Uses standard Windows audio APIs, not Teams integration
- No modification of Teams software or data
- Independent application with separate functionality

#### Enterprise Considerations
- Check with IT department before use in corporate environment
- May be subject to company recording policies
- Consider data governance requirements
- Ensure compliance with industry regulations (HIPAA, SOX, etc.)

---

## 7. FAQ

### 7.1 General Questions

**Q: Do I need to install anything else to use this application?**
A: The application includes all necessary components. You may need to install Visual C++ Redistributables if not already present.

**Q: Can I use this with Teams in a web browser?**
A: The application works best with the Teams desktop application. Web browser support is limited due to audio routing differences.

**Q: Does this work with other video conferencing apps?**
A: Yes! The application captures system audio, so it works with Zoom, WebEx, Google Meet, and any other application that plays audio through your speakers.

**Q: How accurate is the transcription?**
A: Accuracy depends on audio quality and speech recognition provider. Expect 85-95% accuracy with clear audio and Azure/Google providers.

### 7.2 Technical Questions

**Q: Why do I need an API key for some speech providers?**
A: Cloud speech recognition services require authentication and billing. Local Windows Speech Recognition doesn't require an API key but has lower accuracy.

**Q: How much internet bandwidth does this use?**
A: Cloud providers typically use 50-100 KB per minute of audio. Total usage depends on meeting length and audio quality settings.

**Q: Can I use this offline?**
A: Yes, by selecting Windows Speech Recognition as your provider. However, accuracy will be lower than cloud-based services.

**Q: What audio formats are supported?**
A: The application captures audio directly from Windows, so it supports any audio that plays through your speakers/headphones.

### 7.3 Troubleshooting Questions

**Q: The transcription is very delayed. How can I improve this?**
A: Try lowering audio quality, improving internet connection, or switching to a faster speech provider like Azure.

**Q: Some words are consistently wrong. Can I fix this?**
A: Most speech providers improve with usage. You can also use the text search and replace function to fix common errors.

**Q: The application uses too much CPU. What can I do?**
A: Lower audio quality to Medium or Low, close other applications, or switch to Windows Speech Recognition.

**Q: Can I recover a lost transcript?**
A: Check the backup folder in your data directory. Enable auto-save for future sessions to prevent data loss.

---

## 8. Support

### 8.1 Getting Help

#### Self-Service Resources
1. **User Manual**: This comprehensive guide (start here!)
2. **FAQ Section**: Quick answers to common questions
3. **Troubleshooting Guide**: Step-by-step problem resolution
4. **Video Tutorials**: Available on the support website
5. **Community Forums**: Connect with other users

#### Documentation Updates
- Check for updated documentation with each software release
- Subscribe to notifications for important updates
- Bookmark the online help portal for latest information

### 8.2 Technical Support

#### Before Contacting Support
Please try these steps first:
1. Check this user manual for solutions
2. Restart the application
3. Check system requirements
4. Update to the latest version
5. Review error logs in the application data folder

#### Contact Information
- **Email Support**: support@teamstranscription.com
- **Response Time**: 24-48 hours for standard issues
- **Priority Support**: Available for enterprise customers
- **Support Hours**: Monday-Friday, 9 AM - 5 PM EST

#### What to Include in Support Requests
1. **Application Version**: Found in Help → About
2. **Operating System**: Windows version and build number
3. **Error Messages**: Exact text of any error messages
4. **Steps to Reproduce**: What you were doing when the problem occurred
5. **Log Files**: Located in `%APPDATA%\TeamsTranscription\logs`

### 8.3 Feature Requests and Feedback

#### Suggesting Improvements
We welcome your feedback and suggestions:
- **Feature Requests**: Submit through our online portal
- **Bug Reports**: Use the built-in reporting tool
- **User Experience Feedback**: Help us improve the interface
- **Integration Requests**: Suggest new speech providers or export formats

#### Roadmap and Updates
- **Release Notes**: Detailed change information with each update
- **Roadmap**: Future feature plans (updated quarterly)
- **Beta Program**: Early access to new features
- **User Advisory Board**: Opportunity to influence development priorities

### 8.4 Community Resources

#### User Community
- **Forums**: Connect with other users, share tips and tricks
- **Knowledge Base**: Community-contributed solutions and guides
- **Templates**: Share and download export templates
- **Best Practices**: Learn from experienced users

#### Training and Certification
- **Webinars**: Regular training sessions for new features
- **Documentation**: Comprehensive admin and user guides
- **Certification Program**: For enterprise administrators
- **Custom Training**: Available for large organizations

---

## Document Information

**Document Version:** 2.0.0  
**Last Updated:** July 2025  
**Document Owner:** Product Documentation Team  
**Review Cycle:** Quarterly  

### Feedback and Improvements

This user manual is continuously improved based on user feedback. If you:
- Find unclear instructions
- Encounter missing information
- Have suggestions for improvement
- Need additional screenshots or examples

Please contact our documentation team at docs@teamstranscription.com

### Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | June 2025 | Initial user manual |
| 1.5 | June 2025 | Added troubleshooting section |
| 2.0 | July 2025 | Complete rewrite for v2.0 application |

---

*© 2025 Teams Transcription Application. All rights reserved. This manual is provided for informational purposes only. Features and procedures may vary between software versions.*