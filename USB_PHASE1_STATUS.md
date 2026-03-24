# USB Loading Phase 1: Hardcoded Test Implementation

## Status: ✅ COMPLETE (Code Ready for Testing)

## Overview

Phase 1 adds a hardcoded USB game test that can be triggered via button combo during launcher startup. This allows testing USB loading functionality independently before building a full game browser UI.

## Files Created

### 1. launcher/source/usb_test.cpp
Hardcoded USB game test implementation.

**Key Features:**
- Tests with hardcoded game ID (default: "RSBE01" - Super Smash Bros. Brawl)
- 4-step test procedure:
  1. Check USB availability (d2x-cios detection)
  2. Enable WBFS mode for test game
  3. Verify mode change
  4. Read and verify disc ID from USB
- Comprehensive error messages and troubleshooting hints
- Detailed console output for debugging

**Configurable Values:**
```cpp
#define TEST_GAME_ID "RSBE01"  // Change this to your game
#define TEST_DEVICE  USB_DEVICE_USB  // 0=USB, 1=SD
```

**Alternative Game IDs Provided:**
- "RMCE01" - Mario Kart Wii
- "SMNE01" - Super Mario Galaxy
- "RSPP01" - Wii Sports Resort

## Files Modified

### 1. launcher/source/menu_main.cpp
Added debug trigger to MENUINIT_CHECKBUTTONS() macro.

**Trigger Combinations:**
- **Wiimote**: Hold 1 + 2 buttons
- **Classic Controller**: Hold L + R triggers
- **GameCube Controller**: Hold L + R triggers

**Integration:**
- Checks button combo at each MENUINIT_CHECKBUTTONS() call
- Halts GUI and displays "USB DEBUG TEST"
- Runs USB_TestHardcodedGame()
- Waits for user confirmation before continuing

**Works Throughout Launcher:**
- During disc mounting (MenuMount)
- During disc reading (MenuInit)
- Any menu using MENUINIT_CHECKBUTTONS()

### 2. launcher/include/usb.h
Added USB_TestHardcodedGame() function declaration with documentation.

## Debug Trigger Details

### Why No DRC Input?

DRC (Wii U GamePad) input is HAI-IOS specific and won't work with standalone d2x-cios. Using standard Wii controls ensures the test works on:
- ✅ Standalone d2x-cios (IOS 56)
- ✅ HAI-IOS with d2x-cios
- ✅ Any Wii with d2x-cios installed

### Button Combo Requirements

All button checks use standard libogc functions:
- `WPAD_ScanPads()` - Scan Wiimote/Classic Controller
- `PAD_ScanPads()` - Scan GameCube controllers
- `WPAD_ButtonsHeld()` - Check Wiimote button state
- `PAD_ButtonsHeld()` - Check GameCube button state

**Implementation:**
```cpp
u32 wpad_held = WPAD_ButtonsHeld(0) | WPAD_ButtonsHeld(1) |
                WPAD_ButtonsHeld(2) | WPAD_ButtonsHeld(3);
u32 gc_held = PAD_ButtonsHeld(0) | PAD_ButtonsHeld(1) |
              PAD_ButtonsHeld(2) | PAD_ButtonsHeld(3);

// Check all controller types
if ((wpad_held & (WPAD_BUTTON_1 | WPAD_BUTTON_2)) == (WPAD_BUTTON_1 | WPAD_BUTTON_2) ||
    (wpad_held & (WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R)) ==
                 (WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R) ||
    (gc_held & (PAD_TRIGGER_L | PAD_TRIGGER_R)) == (PAD_TRIGGER_L | PAD_TRIGGER_R))
{
    // Trigger USB test
}
```

### Multiple Entry Points

The debug trigger is available at several points:
1. **During SD/USB mounting** - Test before disc is even inserted
2. **During disc loading** - Test after disc detection
3. **During disc parsing** - Test before XML configs are loaded

This allows testing USB functionality at various stages of launcher initialization.

## Test Procedure

### Prerequisites

1. **IOS Requirement**: d2x-cios installed (standalone or in HAI-IOS)
2. **USB Drive**: FAT32 formatted with game in WBFS format
3. **Game Structure**:
   ```
   /wbfs/
   └── Super Smash Bros. Brawl [RSBE01]/
       └── RSBE01.wbfs
   ```
4. **Configuration**: Edit TEST_GAME_ID in usb_test.cpp to match your game

### Steps

1. **Launch HAI-Riivolution**
   - Boot from Homebrew Channel
   - Wait for launcher to initialize

2. **Trigger USB Test**
   - Hold 1+2 on Wiimote, OR
   - Hold L+R on Classic Controller/GameCube controller
   - Hold for ~1 second until "USB DEBUG TEST" appears

3. **Observe Console Output**
   ```
   ========================================
     USB Hardcoded Game Test (Phase 1)
   ========================================
   Game ID: RSBE01
   Device:  USB

   d2x-cios detected - USB loading available

   [1/4] Checking current USB mode...
         Current mode: 0 (None - disc mode)

   [2/4] Setting WBFS mode for RSBE01...
         WBFS mode enabled successfully

   [3/4] Verifying mode change...
         New mode: 1 (WBFS mode) - CORRECT

   [4/4] Reading disc ID from USB...
         Disc ID read: RSBE01
         Expected:     RSBE01

   SUCCESS: Disc ID matches!

   ========================================
     Test Complete
   ========================================

   Game RSBE01 is ready to launch from USB.
   ```

4. **Press A to Continue**
   - Returns to normal launcher operation
   - USB mode remains active (game ready to launch)

### Expected Results

**Success Case:**
- All 4 test steps complete
- Disc ID matches expected game ID
- "SUCCESS: Disc ID matches!" message
- Game is ready to launch from USB

**Error Cases:**

1. **d2x-cios Not Detected:**
   ```
   ERROR: USB loading not available!
   d2x-cios was not detected at startup.
   ```
   Solution: Install d2x-cios

2. **USB Drive Not Found:**
   ```
   ERROR: Failed to set WBFS mode (ret=-6)
   Common causes:
   - USB drive not inserted or not detected
   - Game not found in /wbfs/ directory
   ```
   Solution: Check USB drive and game structure

3. **Disc Read Error:**
   ```
   ERROR: Failed to read disc ID (ret=-1)
   This usually means:
   - Game file is corrupted or invalid
   - USB drive communication error
   ```
   Solution: Re-copy game or try different USB drive

## Next Steps

### After Successful Test

If the hardcoded test succeeds:
1. ✅ USB loading is confirmed working
2. ✅ d2x-cios DIP plugin is functional
3. ✅ Handler chain (Riivolution → d2x → IOS) is working

**Proceed to:**
- Test game launch from USB
- Verify Riivolution patches work with USB games
- Begin Phase 2: Game browser UI implementation

### If Test Fails

Troubleshooting order:
1. Verify d2x-cios installation (check IOS version)
2. Test USB drive in other USB loaders (USBLoaderGX)
3. Check game file integrity
4. Try different USB port (port 0 preferred)
5. Check console output for specific error codes

## Integration with Existing Features

### Riivolution Compatibility

The USB test does NOT interfere with:
- ✅ Normal disc loading
- ✅ Riivolution patch XML parsing
- ✅ Game launching
- ✅ Exit/shutdown functions

After USB test:
- USB mode can stay active
- Normal Riivolution launch will use USB game
- Riivolution patches apply on top of USB data

### Mode Persistence

**Important**: USB mode is NOT persistent across launcher restarts.
- USB mode resets when launcher exits
- Must re-enable USB mode (via test or future game browser)
- This is by design (d2x-cios behavior)

## Build Requirements

**New Source File:**
- `launcher/source/usb_test.cpp` must be compiled
- Makefile should automatically detect it (uses wildcard *.cpp)

**Dependencies:**
- usb.h, usb.cpp (Phase 0)
- wdvd.h (disc reading)
- init.h (PressA function)
- Standard libogc functions

**No Additional Libraries Required**

## Testing Matrix

| Scenario | Expected Behavior | Status |
|----------|------------------|---------|
| d2x-cios present + USB drive | Test succeeds | ⚠️ Untested |
| d2x-cios present + no USB | Friendly error | ⚠️ Untested |
| No d2x-cios | Clear error message | ⚠️ Untested |
| Wrong game ID | Disc ID mismatch warning | ⚠️ Untested |
| Corrupted WBFS | Read error message | ⚠️ Untested |
| Multiple controllers | All combos work | ⚠️ Untested |

## Known Limitations

1. **Hardcoded Game ID**: Must edit source to test different games
2. **Single Device**: Tests USB or SD, not both (configurable)
3. **No Game List**: Cannot browse available games
4. **Manual Trigger**: Must remember button combo

**These will be addressed in Phase 2 (Game Browser UI)**

## Future Enhancements (Phase 2+)

- USB game browser (list all games on USB)
- SD card game browser
- Dynamic game selection
- Save last selected game
- Game metadata display (title, size, etc.)
- Multiple device support
- Automatic USB detection

## Commit Message

Suggested commit for this phase:

```
[LAUNCHER/FEAT] Add USB hardcoded test with button combo trigger

Implement Phase 1 of USB loading:
- Add usb_test.cpp with hardcoded game test (default: RSBE01)
- Add debug trigger: 1+2 (Wiimote), L+R (Classic/GC)
- Integrate into MENUINIT_CHECKBUTTONS() macro
- 4-step test: check mode, enable WBFS, verify, read disc ID
- Comprehensive error messages and troubleshooting
- Works with standalone d2x-cios (no HAI-IOS specific input)

Testing:
- Trigger available at multiple launcher stages
- No interference with normal operation
- USB mode persists after test until launcher exit
```

## References

- USB_LOADING_PLAN.md - Overall implementation plan
- USB_IMPLEMENTATION_STATUS.md - Phase 0 status
- USB_TESTING_GUIDE.md - Testing with standalone d2x-cios
- d2x-cios documentation - WBFS mode details
