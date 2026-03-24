# USB Loading Testing Guide

## Overview

This guide explains how to test USB loading functionality independently from HAI-IOS specific features using standalone d2x-cios.

## Why Separate USB Testing?

- **Isolation**: Test USB loading without HAI-IOS complexity
- **Faster iteration**: Don't need to rebuild HAI-IOS firmware
- **Easier debugging**: Fewer variables in the system
- **Proven base**: d2x-cios is well-tested and stable

## Testing Approach

### Phase 1: Standalone d2x-cios (IOS 56)

Test USB loading with standard d2x-cios before integrating into HAI-IOS.

**Setup**:
1. Install d2x-cios v10-beta53 or later
2. Use IOS 56 base
3. Install to slot 249 or 250
4. Use d2x-cios-installer from [GitHub](https://github.com/Chadderz121/d2x-cios-installer)

**Launcher Configuration**:
- Riivolution launcher will detect d2x-cios automatically
- No HAI-IOS patches needed for initial testing
- Riivolution modules still load dynamically (for disc patching)

**What Works**:
- ✅ USB game loading via d2x-cios
- ✅ Riivolution disc patching (modules loaded by launcher)
- ✅ Both USB and disc loading in same session
- ⚠️ NO HAI-IOS specific features (those require HAI-IOS firmware)

**Test Cases**:
1. USB detection (`USB_Init()` succeeds)
2. USB mode query (`USB_GetMode()` works)
3. Load game from USB (hardcoded test)
4. Load game from disc (verify fallback)
5. Apply Riivolution patches to USB game

### Phase 2: HAI-IOS with d2x-cios

After USB functionality works with standalone d2x, integrate into HAI-IOS.

**Setup**:
1. Build d2x modules into HAI-IOS firmware
2. Flash HAI-IOS to Wii
3. Test all Phase 1 functionality
4. Test HAI-IOS specific features

**What Works**:
- ✅ USB game loading via d2x-cios
- ✅ Riivolution disc patching
- ✅ HAI-IOS specific features
- ✅ Full integration

## Installation: Standalone d2x-cios

### Prerequisites

- Homebrew Channel installed
- SD card or USB drive
- d2x-cios-installer (download from GitHub releases)

### Installation Steps

1. **Download d2x-cios-installer**:
   - Get latest release from: https://github.com/Chadderz121/d2x-cios-installer/releases
   - Extract to `apps/d2x-cios-installer/` on SD card

2. **Prepare IOS files** (optional, installer can download):
   - IOS 56 WAD (base)
   - d2x v10-beta53 or later

3. **Run installer**:
   - Launch from Homebrew Channel
   - Select:
     - Base IOS: **56**
     - Install to slot: **249** (or 250)
     - Version: **v10-beta53** or later
     - NAND: **No** (unless you know what you're doing)

4. **Install**:
   - Confirm installation
   - Wait for completion
   - Return to HBC

5. **Verify**:
   - Check for "Installation complete" message
   - IOS 249 should now be d2x-cios

### Configuration

d2x-cios installs with default settings that work for HAI-Riivolution:
- USB 2.0 support (EHCI module)
- WBFS mode support
- Fraglist support
- FAT32/NTFS game access

No additional configuration needed.

## USB Drive Preparation

### Format USB Drive

**Recommended**:
- Format: FAT32
- Allocation size: 32KB (for large drives)
- Label: "WIIUSB" (optional)

### Game Organization

**WBFS Format** (recommended):
```
/wbfs/
├── Game Name [GAMEID]/
│   └── GAMEID.wbfs
├── Another Game [ANOTHE]/
│   └── ANOTHE.wbfs
└── ...
```

**Example**:
```
/wbfs/
├── Super Smash Bros. Brawl [RSBE01]/
│   └── RSBE01.wbfs
├── Mario Kart Wii [RMCE01]/
│   └── RMCE01.wbfs
```

**Split WBFS** (for FAT32 4GB limit):
```
/wbfs/
├── Large Game [LARGE1]/
│   ├── LARGE1.wbfs
│   ├── LARGE1.z00
│   ├── LARGE1.z01
│   └── ...
```

### Creating WBFS Files

**Using Wii Backup Manager** (Windows):
1. Launch Wii Backup Manager
2. Add disc images (ISO)
3. Select drive (USB)
4. Format: WBFS file
5. Transfer games

**Using wit** (Linux/macOS):
```bash
# Convert ISO to WBFS
wit copy game.iso game.wbfs

# Copy to USB
mkdir -p /mnt/usb/wbfs/"Game Name [GAMEID]"/
cp game.wbfs /mnt/usb/wbfs/"Game Name [GAMEID]"/GAMEID.wbfs
```

## Testing Procedure

### Step 1: Verify d2x-cios Installation

1. Boot Wii with d2x-cios installed
2. Launch HAI-Riivolution
3. Check console output:
   ```
   USB: Initializing USB support...
   USB: /dev/usb2 found (fd=X)
   USB: d2x-cios DIP plugin detected!
   USB: USB loading is AVAILABLE
   ```

### Step 2: Test USB Detection

1. Insert USB drive with games
2. Observe console output
3. Check if d2x-cios detects USB storage

### Step 3: Test Hardcoded Game (Phase 1)

1. Edit `usb_test.cpp` with your game ID:
   ```cpp
   #define TEST_GAME_ID "RSBE01"  // Your game ID
   ```
2. Build launcher
3. Press L+R on GamePad
4. Verify output:
   ```
   === USB Hardcoded Game Test ===
   Game ID: RSBE01
   WBFS mode set successfully
   Disc ID: RSBE01
   SUCCESS: Disc ID matches!
   ```

### Step 4: Test Game Launch (Phase 2)

1. Use normal launch process
2. Game should boot from USB
3. Verify USB activity light

### Step 5: Test Riivolution Patches (Phase 2)

1. Create Riivolution XML config
2. Enable patches (file replacements)
3. Launch game from USB
4. Verify patches are applied

### Step 6: Test Disc Fallback

1. Remove USB drive
2. Insert disc
3. Verify disc loading still works
4. Check console: "USB: Operating in disc-only mode"

## Troubleshooting

### USB Not Detected

**Symptom**: `/dev/usb2 not found`

**Solutions**:
- Verify d2x-cios is installed (check in System Menu → Wii Options → Wii Settings → About)
- Reinstall d2x-cios
- Check IOS version (should be 249 or 250 with d2x)

### Games Not Found

**Symptom**: Game list empty or specific game not loading

**Solutions**:
- Check USB drive format (FAT32 recommended)
- Verify game folder structure: `/wbfs/GameName [GAMEID]/GAMEID.wbfs`
- Ensure game ID matches folder name
- Try different USB port (prefer port 0, closest to edge)

### Game Won't Boot

**Symptom**: Black screen or crash when launching

**Solutions**:
- Verify WBFS file is not corrupted
- Try re-converting from ISO
- Check game region matches Wii region
- Test same game from disc to rule out game issues

### Riivolution Patches Not Applied

**Symptom**: Game loads but patches don't work

**Solutions**:
- Verify patches work with disc first
- Check Riivolution XML syntax
- Ensure SD card is inserted (patches read from SD)
- Check console output for patch errors

## Development Workflow

1. **Initial Development**: Use standalone d2x-cios
   - Fast iteration
   - Easy debugging
   - Proven base

2. **USB Feature Complete**: Integrate into HAI-IOS
   - Build d2x modules into firmware
   - Test full integration
   - Verify no regressions

3. **Production**: HAI-IOS with d2x-cios
   - Single IOS installation
   - All features in one package
   - Easier for end users

## Comparison: Standalone vs HAI-IOS

| Feature | Standalone d2x | HAI-IOS + d2x |
|---------|---------------|---------------|
| USB Loading | ✅ | ✅ |
| Disc Patching | ✅ (via modules) | ✅ |
| GamePad Input | ✅ | ✅ |
| HAI Features | ❌ | ✅ |
| Installation | Easy | Complex |
| Updates | Independent | Firmware update |
| Testing | Fast | Slower |
| Production | Not recommended | Recommended |

## Next Steps

After successful testing with standalone d2x-cios:

1. Document any d2x-cios compatibility issues
2. Create list of tested USB drives
3. Coordinate with HAI-IOS firmware build
4. Integrate d2x modules into HAI-IOS
5. Re-test all functionality
6. Release combined HAI-IOS + d2x firmware

## References

- [d2x-cios GitHub](https://github.com/Chadderz121/d2x-cios)
- [d2x-cios Wiki](https://wiibrew.org/wiki/D2x_cIOS)
- [Wii Backup Manager](http://www.wiibackupmanager.co.uk/)
- [wit (Wiimms ISO Tools)](https://wit.wiimm.de/)
- [USB Loading Guide](https://wiibrew.org/wiki/USB_Loader)
