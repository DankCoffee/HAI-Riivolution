# USB Loading Implementation Status

## Phase 0: d2x-cios Detection - ✅ COMPLETE

### Files Created

1. **launcher/include/usb.h**
   - USB API header with d2x-cios IOCTL definitions
   - Functions for detection, mode query, and WBFS control
   - Status: ✅ Complete

2. **launcher/source/usb.cpp**
   - Full USB API implementation
   - d2x-cios detection via `/dev/usb2` and MODE_GET test
   - Graceful fallback to disc-only mode if d2x not present
   - Status: ✅ Complete

### Files Modified

1. **launcher/source/init.cpp**
   - Added `#include "usb.h"`
   - Added `USB_Init()` call in `Initialise()` function
   - Displays USB status on startup
   - Status: ✅ Complete

### Implementation Details

#### Detection Flow

```cpp
USB_Init():
1. Check for /dev/usb2 device
   - If not present: return -1 (disc-only)

2. Open /dev/do (Riivolution DIP)
   - If fails: return -2 (module error)

3. Send IOCTL_DI_MODE_GET (0xFA)
   - If fails: return -3 (d2x not responding)
   - If succeeds: d2x-cios detected!

4. Set g_usb_available = 1
   - All USB functions now enabled
```

#### API Functions

- `USB_Init()` - Detect d2x-cios, must be called first
- `USB_IsAvailable()` - Check if USB loading is available
- `USB_GetMode()` - Query current mode (NONE/WBFS/FRAG)
- `USB_SetWBFSMode(game_id, device)` - Enable USB loading
- `USB_DisableWBFS()` - Return to disc mode
- `USB_GetStatusString()` - Get human-readable status

#### Default Behavior

If d2x-cios is not detected:
- All USB functions return error codes
- Launcher operates in disc-only mode
- No crashes or error dialogs
- Clear console messages explain the situation

### Testing Status

⚠️ **NOT TESTED** - Requires building in devkitPPC Docker container and testing on hardware

#### IOS Requirements for Testing

**Option 1: Standalone d2x-cios (Recommended for Testing)**
- Install d2x-cios with IOS 56 base via d2x-cios-installer
- Provides USB loading without HAI-IOS
- Easier to test USB functionality independently
- Install as IOS 249 or IOS 250 (standard d2x slots)
- Launcher will detect d2x-cios regardless of IOS version

**Option 2: HAI-IOS with d2x-cios (Production)**
- Full HAI-IOS firmware with d2x modules bundled
- Provides both USB loading AND Riivolution patches
- Use this for final integration testing

**Testing Order**:
1. Test with standalone d2x-cios first (isolate USB functionality)
2. Test with HAI-IOS + d2x-cios (full integration)
3. Test with HAI-IOS without d2x-cios (disc-only fallback)

#### Test Plan

1. **Build Test**:
   ```bash
   # In devkitPPC container:
   cd /home/yaakov/HAI-Riivolution/launcher
   make clean
   make -j
   ```
   - Verify usb.cpp compiles
   - Check for linker errors
   - Ensure boot.elf is created

2. **Runtime Test (d2x-cios present)**:
   - Boot on Wii with standalone d2x-cios (IOS 56) OR HAI-IOS + d2x-cios
   - Check console output for:
     ```
     USB: Initializing USB support...
     USB: Checking for /dev/usb2...
     USB: /dev/usb2 found (fd=X)
     USB: Testing d2x-cios DIP plugin via /dev/do...
     USB: d2x-cios DIP plugin detected!
     USB: Current mode: 0
     USB: USB loading is AVAILABLE
     USB: USB loading is available!
     USB: You can load games from USB or SD card
     USB: Status - d2x-cios detected - USB loading available
     ```

3. **Runtime Test (d2x-cios NOT present)**:
   - Boot on Wii with standard IOS (no d2x-cios)
   - Check console output for:
     ```
     USB: Initializing USB support...
     USB: Checking for /dev/usb2...
     USB: /dev/usb2 not found (ret=X)
     USB: d2x-cios USB module not present in firmware
     USB: Defaulting to disc-only mode
     USB: Operating in disc-only mode
     USB: To enable USB loading, ensure d2x-cios is installed
     USB: Status - Disc-only mode (d2x-cios not detected)
     ```
   - Note: This tests graceful fallback to disc-only mode

4. **Functional Test**:
   - Call `USB_IsAvailable()` and verify return value
   - If available, call `USB_GetMode()` and verify response
   - Ensure existing disc loading still works

### Next Steps

**Phase 1: Hardcoded USB Test**

1. Create `launcher/source/usb_test.cpp`:
   - Hardcoded game ID test function
   - USB mode enable/disable
   - Disc ID verification

2. Add debug trigger to `launcher/source/menu_main.cpp`:
   - L+R button combo to trigger test
   - Display test results

3. Test with physical USB drive:
   - Format USB as FAT32
   - Create `/wbfs/GameName [GAMEID]/GAMEID.wbfs`
   - Run hardcoded test
   - Verify disc reads come from USB

**Phase 2: Game Launch Integration**

1. Integrate USB mode with existing launcher
2. Test game boots from USB
3. Verify Riivolution patches apply correctly

**Phase 3: Game Browser UI**

1. Implement USB game list reading
2. Add USB browser menu
3. Full game selection UI

### Known Issues

None yet - awaiting build and hardware testing.

### Build Requirements

- devkitPPC toolchain in Docker container
- All dependencies from README.md
- Clean build recommended after adding new files

### Commit Message

Suggested commit message for this phase:

```
[LAUNCHER/FEAT] Add d2x-cios USB detection and API

Implement Phase 0 of USB loading support:
- Add USB API (usb.h, usb.cpp) with d2x-cios detection
- Detect d2x-cios via /dev/usb2 and MODE_GET IOCTL
- Default to disc-only mode if d2x-cios not present
- Initialize USB support in launcher startup

USB loading is now gracefully available when d2x-cios
is present in HAI-IOS firmware, with automatic fallback
to disc-only operation if not detected.
```

### File Checklist

- [x] launcher/include/usb.h - Created
- [x] launcher/source/usb.cpp - Created
- [x] launcher/source/init.cpp - Modified
- [ ] launcher/Makefile - Verify usb.cpp is picked up automatically
- [ ] Build test in Docker
- [ ] Hardware test with d2x-cios
- [ ] Hardware test without d2x-cios
