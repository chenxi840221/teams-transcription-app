# GUI Layout Verification Guide

## Current Application Status
✅ **Application runs successfully**  
✅ **Azure OpenAI provider loaded (Provider 3)**  
✅ **Window size increased to 1000x650**  
✅ **Dual-panel layout implemented**  

## Expected GUI Layout

### Window Structure:
```
┌─────────────────────────────────────────────────────────────────────────────────┐
│ [Start] [Stop] [Pause] [Settings] [Export] [Clear]                            │
│ Teams Status: Not Connected              [████████████]                       │
│                                                                                │
│ Transcription                           Debug Log                             │
│ ┌─────────────────────────────┐         ┌─────────────────────────────┐        │
│ │Transcription will appear    │         │Debug information will       │        │
│ │here...                      │         │appear here...               │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ │                             │         │                             │        │
│ └─────────────────────────────┘         └─────────────────────────────┘        │
│ Ready                                                                  [≡]     │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### Panel Specifications:
- **Left Panel (Transcription)**:
  - Position: 10px from left, 105px from top
  - Size: 480px wide × 480px tall
  - Content: Clean transcription text only

- **Right Panel (Debug Log)**:
  - Position: 500px from left, 105px from top  
  - Size: 480px wide × 480px tall
  - Content: Timestamped debug messages

## What Should Be Visible:

### 1. **Application Startup**
After ~3 seconds, the Debug Log should show:
```
[XXXs] Application initialized successfully
[XXXs] Audio capture: Ready
[XXXs] Speech recognition: Ready
```

### 2. **Audio Processing (every 3 seconds)**
Debug Log should show messages like:
```
[XXXs] Azure OpenAI GPT-4o transcription #1 - Processed 1058444 bytes of WAV audio
[XXXs] Azure OpenAI GPT-4o transcription #2 - Processed 1065500 bytes of WAV audio
```

### 3. **Transcription Panel**
- Should remain **clean** with just "Transcription will appear here..."
- **No timestamps or debug messages** should appear here
- Ready for real transcription content

## Troubleshooting Steps:

### If Debug Messages Don't Appear:
1. Check if debug log control is created properly
2. Verify `ID_DEBUG_LOG_EDIT` is correct
3. Check if `UpdateDebugLog()` is being called

### If Layout Looks Wrong:
1. **Window too small**: Should be 1000x650 pixels
2. **Panels overlapping**: Left panel ends at 490px, right starts at 500px
3. **Controls not visible**: Check if they're outside window bounds

### If Still Having Issues:
1. **Test with Clear button**: Should clear both panels
2. **Check window resize**: Panels should stay in position
3. **Verify control IDs**: Make sure no ID conflicts

## Expected Behavior:
- ✅ Two clearly separated panels
- ✅ Clean transcription area (left)
- ✅ Timestamped debug info (right)
- ✅ Audio processing messages appear in debug log every 3 seconds
- ✅ Window is wider to accommodate both panels
- ✅ No overlap between panels

This layout provides the foundation for real Azure OpenAI transcription - actual transcribed speech will appear in the left panel while system information stays in the right panel.