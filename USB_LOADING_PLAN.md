# USB Loading Integration Plan for HAI-Riivolution

## Overview

This plan outlines the integration of d2x-cios USB loading functionality into HAI-Riivolution while maintaining Riivolution's patching capabilities. The goal is to create a handler chain: **Riivolution → d2x-cios → Original IOS**, allowing USB game loading with Riivolution patches applied on top.

---

## Current Architecture

### HAI-IOS Firmware (Pre-boot)

```
HAI-IOS Custom Firmware (IOS 56 base)
    ↓ (modules bundled in fw.img)
d2x-cios modules loaded at IOS boot:
    1. ehci-module → USB 2.0 host controller
    2. usb-module → /dev/usb2 (USB storage driver + WBFS)
    3. dip-plugin → patches /dev/di handlers in-place for USB disc emulation
```

**IMPORTANT**: These modules are already active BEFORE any PowerPC application runs.

### HAI-Riivolution Module Loading (haxx.cpp)

```
PowerPC Application Starts
    ↓ Haxx_Init() → load_module_code()
Riivolution Module Loading Sequence:
    1. filemodule_elf → /dev/file (filesystem patches)
    2. dipmodule_elf → /dev/do (disc interface patches, proxies to /dev/di)
    3. megamodule_elf → /dev/mega (debugger, optional)
```

### Riivolution DIP Module (dipmodule/)

```
ProxiIOS::DIP::DIP
    extends ProxyModule("/dev/do", "/dev/di")

HandleIoctl/HandleIoctlv:
    - Custom commands (Allocate, Patch, etc.) → handled internally
    - Standard DI commands → ForwardIoctl() → os_ioctl(ProxyHandle="/dev/di", ...)
```

**Key Insight**: Riivolution DIP proxies to `/dev/di`, which is already patched by d2x-cios DIP plugin. The handler chain already exists!

---

## Target Architecture

### Actual Module State at Runtime

```
IOS Boot Time:
    HAI-IOS fw.img loads
        ↓
    d2x-cios modules initialize:
        - EHCI module active
        - USB module registers /dev/usb2
        - DIP plugin patches /dev/di in-place
    ↓
PowerPC Application Starts
    ↓
    Haxx_Init() loads Riivolution modules:
        - filemodule registers /dev/file
        - dipmodule registers /dev/do (proxies to /dev/di)
        - megamodule registers /dev/mega (optional)
```

### Handler Chain (Already Implemented!)

```
Application calls: IOS_Open("/dev/do", ...)
    ↓
Riivolution DIP (/dev/do) [Loaded by PowerPC app]
    - Handles Riivolution-specific commands:
        * 0x46 (Allocate) - memory allocation
        * 0x47 (Free) - memory deallocation
        * 0x50 (Patch) - disc patching
        * 0x51 (Reset) - reset patches
        * 0x52 (Query) - query patch status
    - Forwards all other commands via os_ioctl(ProxyHandle="/dev/di", ...)
    ↓
d2x-cios DIP Plugin [Already patched into /dev/di at IOS boot]
    - Intercepts /dev/di handler functions in-place
    - Handles USB loading commands:
        * 0xF4 (WBFS_SET) - enable WBFS mode
        * 0xF5 (WBFS_GET) - query WBFS mode
        * 0xF9 (FRAG_SET) - set fraglist mode
        * 0xFA (MODE_GET) - query emulation mode
        * 0xFE (SAVE_CONFIG) - save persistent config
    - When USB mode active: redirects reads to /dev/usb2
    - Otherwise: calls original /dev/di handlers (stored function pointers)
    ↓
Original /dev/di handlers [Stored as function pointers by d2x]
    - Physical DVD drive access
    - Standard disc operations
```

**Critical Realization**: The handler chain ALREADY EXISTS. d2x-cios is already in the firmware and Riivolution already proxies through it. We just need to USE it!

---

## Implementation Phases

### Phase 0: Verify d2x-cios in IOS Firmware

**PREREQUISITE**: IOS must include d2x-cios modules for USB support.

#### IOS Options

**Option 1: HAI-IOS with d2x-cios (Full Featured)**
- HAI-IOS firmware (fw.img) with d2x-cios modules bundled
- Provides both USB loading AND Riivolution patching
- Riivolution modules loaded dynamically by PowerPC app

**Option 2: Standalone d2x-cios (IOS 56 base) - USB Testing Only**
- Standard d2x-cios installer with IOS 56 base
- Provides USB loading support
- Does NOT include Riivolution patches
- **Use this for testing USB functionality independently**
- Install via: [d2x-cios-installer](https://github.com/Chadderz121/d2x-cios-installer)

#### Detection Code

**Check if d2x modules are present**:
1. Boot IOS and check for devices:
   ```cpp
   // In launcher init code
   int usb_fd = IOS_Open("/dev/usb2", 0);
   if (usb_fd >= 0) {
       printf("d2x USB module present\n");
       IOS_Close(usb_fd);
   }
   ```

2. Check IOS version and patches:
   ```cpp
   u32 ios_ver = IOS_GetVersion();
   u32 ios_rev = IOS_GetRevision();
   printf("IOS: %d rev %d\n", ios_ver, ios_rev);
   ```

#### For HAI-IOS Integration

**If d2x modules are NOT in HAI-IOS firmware**:
- They need to be compiled and bundled into HAI-IOS fw.img
- This is done OUTSIDE this codebase (firmware build process)
- Coordinate with HAI-IOS build system to include:
  - ehci-module.elf
  - usb-module.elf
  - dip-plugin.elf (with IOS 56 patches)

**Recommendation for Development**:
1. Test USB loading first with standalone d2x-cios (IOS 56)
2. Once working, integrate d2x modules into HAI-IOS firmware
3. This separates USB testing from HAI-IOS specific features

---

### Phase 1: Verify Handler Chain Compatibility

Since d2x-cios DIP is already patching `/dev/di` in-place at IOS boot, and Riivolution DIP proxies to `/dev/di`, the chain should already work. We need to verify it doesn't break.

#### 1.1 Test Current Riivolution Functionality

**Goal**: Ensure Riivolution still works with d2x-cios present.

**Tests**:
1. Load game from disc with Riivolution patches
2. Verify file replacements work
3. Verify memory patches work
4. Check for any conflicts or crashes

**Expected Behavior**:
- Riivolution `/dev/do` receives commands
- Forwards standard DI commands to `/dev/di`
- d2x-cios DIP intercepts these at `/dev/di`
- Since USB mode is NOT set, d2x passes through to original handlers
- Game loads from disc normally with Riivolution patches applied

#### 1.2 Verify d2x-cios Commands Pass Through

**Goal**: Ensure d2x-specific IOCTLs (0xF4-0xFF) reach d2x-cios.

**Test code** (add to launcher):
```cpp
// Test that d2x commands pass through Riivolution proxy
int TestD2xPassthrough() {
    int fd = IOS_Open("/dev/do", 0);
    if (fd < 0) return -1;

    u32 inbuf[8] ATTRIBUTE_ALIGN(32);
    u32 outbuf[8] ATTRIBUTE_ALIGN(32);

    // Send MODE_GET (0xFA) to check d2x-cios responds
    inbuf[0] = 0xFA << 24;
    int ret = IOS_Ioctl(fd, 0xFA, inbuf, sizeof(inbuf), outbuf, sizeof(outbuf));

    IOS_Close(fd);
    return ret; // Should be 0 if d2x is present and responding
}
```

**If this fails**: Riivolution DIP may be blocking d2x commands. Need to ensure Riivolution forwards unknown commands properly.

---

### Phase 2: Add PowerPC-Side USB Interface

Since d2x-cios is already in the firmware, we just need PowerPC code to communicate with it.

#### 2.1 Create USB API (launcher/source/usb.cpp)

```cpp
#include "usb.h"
#include <gccore.h>
#include <ogc/ios.h>
#include <string.h>

// d2x-cios IOCTL commands
#define IOCTL_DI_WBFS_SET   0xF4
#define IOCTL_DI_WBFS_GET   0xF5
#define IOCTL_DI_FRAG_SET   0xF9
#define IOCTL_DI_MODE_GET   0xFA
#define IOCTL_DI_SAVE_CONFIG 0xFE

// Modes
#define MODE_NONE   0
#define MODE_WBFS   1
#define MODE_FRAG   2

// Devices
#define DEVICE_USB  0
#define DEVICE_SD   1

int USB_GetMode() {
    int fd = IOS_Open("/dev/do", 0);
    if (fd < 0) return -1;

    u32 inbuf[8] ATTRIBUTE_ALIGN(32);
    u32 outbuf[8] ATTRIBUTE_ALIGN(32);

    inbuf[0] = IOCTL_DI_MODE_GET << 24;
    int ret = IOS_Ioctl(fd, IOCTL_DI_MODE_GET, inbuf, sizeof(inbuf),
                        outbuf, sizeof(outbuf));

    IOS_Close(fd);
    return (ret >= 0) ? outbuf[0] : -1;
}

int USB_SetWBFSMode(const char* game_id, int device) {
    int fd = IOS_Open("/dev/do", 0);
    if (fd < 0) return -1;

    u32 inbuf[8] ATTRIBUTE_ALIGN(32);
    memset(inbuf, 0, sizeof(inbuf));

    inbuf[0] = IOCTL_DI_WBFS_SET << 24;
    inbuf[1] = device; // 0=USB, 1=SD
    memcpy(&inbuf[2], game_id, 6);

    int ret = IOS_Ioctl(fd, IOCTL_DI_WBFS_SET, inbuf, sizeof(inbuf), NULL, 0);

    IOS_Close(fd);
    return ret;
}

int USB_DisableWBFS() {
    int fd = IOS_Open("/dev/do", 0);
    if (fd < 0) return -1;

    u32 inbuf[8] ATTRIBUTE_ALIGN(32);
    memset(inbuf, 0, sizeof(inbuf));

    inbuf[0] = IOCTL_DI_WBFS_SET << 24;
    inbuf[1] = 0xFF; // Invalid device = disable

    int ret = IOS_Ioctl(fd, IOCTL_DI_WBFS_SET, inbuf, sizeof(inbuf), NULL, 0);

    IOS_Close(fd);
    return ret;
}
```

#### 2.2 Create USB Header (launcher/include/usb.h)

```cpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Query current USB mode
int USB_GetMode();

// Enable WBFS mode for specific game ID
// game_id: 6-character game ID (e.g. "RSBE01")
// device: 0=USB, 1=SD
int USB_SetWBFSMode(const char* game_id, int device);

// Disable USB loading (return to disc mode)
int USB_DisableWBFS();

#ifdef __cplusplus
}
#endif
```

---

### Phase 3: Hardcoded USB Game Loading Test

#### 3.1 Hardcoded Test Implementation

**Goal**: Test USB loading with a hardcoded game path before building full UI.

**Create launcher/source/usb_test.cpp**:
```cpp
#include "usb.h"
#include "launcher.h"
#include <stdio.h>
#include <gccore.h>

// Hardcoded test values - change these for your setup
#define TEST_GAME_ID "RSBE01"  // Super Smash Bros. Brawl
#define TEST_DEVICE 0          // 0=USB, 1=SD

int USB_TestHardcodedGame() {
    printf("=== USB Hardcoded Game Test ===\n");
    printf("Game ID: %s\n", TEST_GAME_ID);
    printf("Device: %s\n", TEST_DEVICE == 0 ? "USB" : "SD");

    // 1. Check current mode
    int mode = USB_GetMode();
    printf("Current mode: %d\n", mode);

    // 2. Enable WBFS mode
    printf("Setting WBFS mode...\n");
    int ret = USB_SetWBFSMode(TEST_GAME_ID, TEST_DEVICE);
    if (ret < 0) {
        printf("FAILED to set WBFS mode: %d\n", ret);
        return ret;
    }
    printf("WBFS mode set successfully\n");

    // 3. Verify mode changed
    mode = USB_GetMode();
    printf("New mode: %d (should be 1 for WBFS)\n", mode);

    // 4. Try to read disc ID
    printf("Reading disc ID...\n");
    u32 disc_id[8] ATTRIBUTE_ALIGN(32);
    ret = WDVD_ReadDiskId(disc_id);
    if (ret < 0) {
        printf("FAILED to read disc ID: %d\n", ret);
        USB_DisableWBFS(); // Clean up
        return ret;
    }

    printf("Disc ID: %.6s\n", (char*)disc_id);
    printf("Expected: %.6s\n", TEST_GAME_ID);

    if (memcmp(disc_id, TEST_GAME_ID, 6) != 0) {
        printf("WARNING: Disc ID mismatch!\n");
    } else {
        printf("SUCCESS: Disc ID matches!\n");
    }

    printf("=== Test Complete ===\n");
    printf("Game is ready to launch from USB\n");

    return 0;
}
```

#### 3.2 Add Debug Menu Option

**Modify launcher/source/menu_main.cpp**:
```cpp
// Add at top
#include "usb.h"
extern int USB_TestHardcodedGame();

// In main menu loop (add debug combo)
void MainMenuLoop() {
    while (true) {
        // ... existing menu code ...

        // Debug combo: L + R buttons = USB test
        if (WiiDRC_Inited() && WiiDRC_Connected()) {
            WiiDRC_ScanPads();
            u32 drc_buttons = WiiDRC_ButtonsDown();

            if ((drc_buttons & WIIDRC_BUTTON_L) &&
                (drc_buttons & WIIDRC_BUTTON_R)) {
                printf("\n*** USB TEST MODE ***\n");
                USB_TestHardcodedGame();
                PressA(); // Wait for user
            }
        }

        // ... rest of menu code ...
    }
}
```

#### 3.3 Test Procedure

1. **Prepare USB drive**:
   - Format USB drive as FAT32
   - Create `/wbfs/` directory
   - Place game in WBFS format: `/wbfs/GameName [GAMEID]/GAMEID.wbfs`
   - Example: `/wbfs/Super Smash Bros. Brawl [RSBE01]/RSBE01.wbfs`

2. **Boot Riivolution**:
   - Insert USB drive
   - Boot HAI-Riivolution launcher

3. **Trigger test**:
   - Press L + R on GamePad
   - Watch console output

4. **Expected output**:
   ```
   === USB Hardcoded Game Test ===
   Game ID: RSBE01
   Device: USB
   Current mode: 0
   Setting WBFS mode...
   WBFS mode set successfully
   New mode: 1 (should be 1 for WBFS)
   Reading disc ID...
   Disc ID: RSBE01
   Expected: RSBE01
   SUCCESS: Disc ID matches!
   === Test Complete ===
   Game is ready to launch from USB
   ```

5. **Launch game**:
   - Use normal Riivolution launch process
   - Game should boot from USB with Riivolution patches applied

---

### Phase 4: Full USB Game Browser

Once hardcoded loading works, implement full game browser UI.

#### 4.1 Game List Reading

**Add to launcher/source/usb.cpp**:
```cpp
#include "usb.h"
#include <gccore.h>
#include <ogc/ios.h>

#define IOCTL_DI_WBFS_SET 0xF4

int USB_Init() {
    // Already initialized via d2x modules in Haxx_Init()
    return 0;
}

int USB_SetWBFSMode(const char* game_id) {
    int di_fd = IOS_Open("/dev/do", 0);
    if (di_fd < 0)
        return di_fd;

    u32 inbuf[8] ATTRIBUTE_ALIGN(32);
    inbuf[0] = IOCTL_DI_WBFS_SET << 24;
    inbuf[1] = 0;  // USB device
    memcpy(&inbuf[2], game_id, 6);

    int ret = IOS_Ioctl(di_fd, IOCTL_DI_WBFS_SET, inbuf, sizeof(inbuf), NULL, 0);

    IOS_Close(di_fd);
    return ret;
}

int USB_GetGameList(GameEntry* games, int max_games) {
    // TODO: Implement game list reading
    // Could use WBFS module directly or read disc headers
    return 0;
}
```

#### 4.2 Update launcher UI

**Add USB game browser to launcher/source/menu_main.cpp**:
```cpp
// Add new menu option
enum MenuOption {
    MENU_LOAD_DISC,
    MENU_LOAD_USB,      // NEW
    MENU_LOAD_SD,       // NEW
    MENU_SETTINGS,
    MENU_EXIT
};

void LoadFromUSB() {
    printf("Scanning USB for games...\n");

    GameEntry games[MAX_GAMES];
    int count = USB_GetGameList(games, MAX_GAMES);

    if (count <= 0) {
        printf("No games found on USB\n");
        return;
    }

    // Display game list, user selects game
    int selected = DisplayGameList(games, count);

    // Set WBFS mode for selected game
    USB_SetWBFSMode(games[selected].id);

    // Launch game normally (Riivolution patches will apply)
    LaunchGame();
}
```

---

### Phase 5: Testing and Hardcoded Location

#### 5.1 Hardcoded Test Implementation

For initial testing, implement a hardcoded game loader:

**launcher/source/usb_test.cpp**:
```cpp
#define TEST_GAME_ID "RSBE01"  // Super Smash Bros. Brawl
#define TEST_GAME_PATH "/wbfs/Super Smash Bros. Brawl [RSBE01]/RSBE01.wbfs"

int USB_LoadHardcodedGame() {
    printf("Loading hardcoded game: %s\n", TEST_GAME_ID);

    // 1. Initialize USB (already done in Haxx_Init)

    // 2. Set WBFS mode for specific game
    int ret = USB_SetWBFSMode(TEST_GAME_ID);
    if (ret < 0) {
        printf("Failed to set WBFS mode: %d\n", ret);
        return ret;
    }

    // 3. Verify disc can be read
    u32 disc_id[8] ATTRIBUTE_ALIGN(32);
    ret = WDVD_ReadDiskId(disc_id);
    if (ret < 0) {
        printf("Failed to read disc ID: %d\n", ret);
        return ret;
    }

    printf("Disc ID: %.6s\n", (char*)disc_id);

    // 4. Launch game (Riivolution patches will apply)
    return LaunchGame();
}
```

**Add debug menu option**:
```cpp
// In menu_main.cpp
if (buttons & WIIDRC_BUTTON_L && buttons & WIIDRC_BUTTON_R) {
    // Secret debug combo: L+R
    USB_LoadHardcodedGame();
}
```

#### 5.2 Test Cases

1. **USB detection**:
   - Insert USB drive with WBFS game
   - Verify `usbstorage_Init()` succeeds
   - Check logs for USB enumeration

2. **WBFS open**:
   - Call `WBFS_OpenDisc()` with test game ID
   - Verify disc header can be read
   - Check logs for WBFS partition detection

3. **Disc read**:
   - Read disc ID via DI commands
   - Read first sector of game partition
   - Verify data matches expected game

4. **Game launch**:
   - Launch hardcoded game from USB
   - Verify game boots correctly
   - Test without Riivolution patches first

5. **Riivolution patches**:
   - Apply simple file replacement patch
   - Launch game from USB with patches
   - Verify patches are applied correctly

---

## Implementation Order (Simplified!)

### Milestone 1: Verify d2x-cios Presence
1. Add diagnostic code to check for `/dev/usb2`
2. Test `USB_GetMode()` to verify d2x DIP responds
3. Verify handler chain doesn't break existing Riivolution functionality
4. **Duration**: 1-2 hours

### Milestone 2: PowerPC USB API
1. Create `usb.h` and `usb.cpp` with d2x-cios interface
2. Implement `USB_GetMode()`, `USB_SetWBFSMode()`, `USB_DisableWBFS()`
3. Add to launcher build system
4. **Duration**: 2-3 hours

### Milestone 3: Hardcoded USB Test
1. Create `usb_test.cpp` with hardcoded game loader
2. Add L+R debug combo to menu
3. Test with physical USB drive and game
4. Verify disc ID reading works
5. **Duration**: 3-4 hours

### Milestone 4: Game Launch from USB
1. Integrate USB mode with existing game launcher
2. Test game boots from USB
3. Verify Riivolution patches apply correctly
4. Test multiple games
5. **Duration**: 4-6 hours

### Milestone 5: Game Browser UI
1. Implement USB game list reading
2. Add USB browser menu option
3. Create game selection UI
4. Add SD card support
5. **Duration**: 8-12 hours

### Milestone 6: Polish and Release
1. Error handling and user feedback
2. USB compatibility testing
3. Documentation
4. Create builds and release
5. **Duration**: 4-6 hours

**Total Estimated Time**: 20-35 hours (vs. 80-120 hours for full module rewrite!)

---

## Technical Challenges and Solutions

### Challenge 1: d2x-cios Module Availability

**Problem**: HAI-IOS firmware may not include d2x-cios modules yet.

**Solution**:
- Check for `/dev/usb2` presence at runtime
- If missing, show error message to user
- Document firmware requirements
- May need to coordinate with HAI-IOS firmware build to add d2x modules

### Challenge 2: Handler Chain Compatibility

**Problem**: Riivolution DIP may not correctly forward d2x-specific IOCTLs (0xF4-0xFF).

**Solution**:
- Verify current Riivolution DIP implementation
- Ensure `ForwardIoctl()` passes through all unknown commands
- Add logging to track IOCTL flow
- Test with `USB_GetMode()` to verify d2x responds

### Challenge 3: Command Format Compatibility

**Problem**: d2x-cios expects specific command buffer format, Riivolution may modify it.

**Solution**:
- Follow d2x-cios command format exactly
- Commands use pattern: `inbuf[0] = (command << 24) | params`
- Ensure buffer alignment (ATTRIBUTE_ALIGN(32))
- Test each IOCTL individually

### Challenge 4: USB Storage Compatibility

**Problem**: Different USB drives have varying compatibility with Wii USB stack.

**Solution**:
- Test with known-compatible drives first
- Implement proper USB error handling
- Add retry logic for USB timeouts
- Document compatible USB drive models
- Consider adding USB compatibility list to UI

### Challenge 5: WBFS File Format

**Problem**: Games must be in WBFS format, users may have ISO files.

**Solution**:
- Document WBFS conversion process
- Recommend tools (Wii Backup Manager, etc.)
- Support both WBFS partition and FAT32 with WBFS files
- Add format detection and helpful error messages

---

## File Structure (Minimal Changes!)

```
HAI-Riivolution/
├── launcher/
│   ├── source/
│   │   ├── usb.cpp                  # USB interface (NEW)
│   │   ├── usb_test.cpp             # Hardcoded test loader (NEW)
│   │   ├── menu_main.cpp            # UI updates (MODIFIED - add USB menu)
│   │   └── launcher.cpp             # Game launch (MODIFIED - USB support)
│   ├── include/
│   │   └── usb.h                    # USB API (NEW)
│   └── Makefile                     # MODIFIED - add usb.cpp, usb_test.cpp
├── dipmodule/                       # NO CHANGES NEEDED
│   └── source/
│       └── dip.cpp                  # Already proxies correctly!
├── filemodule/                      # NO CHANGES
├── megamodule/                      # NO CHANGES
├── libios/                          # NO CHANGES
└── USB_LOADING_PLAN.md              # THIS FILE
```

**Key Insight**: Existing Riivolution modules don't need modification! They already work correctly with d2x-cios.

---

## Alternative Approaches Considered

### Alternative 1: Rewrite d2x-cios as ProxiIOS Modules (Rejected)

- Port d2x-cios modules to ProxiIOS framework
- Load via PowerPC application like Riivolution modules
- **Pros**: More control, consistent architecture
- **Cons**: 80+ hours of work, memory constraints, unnecessary complexity

### Alternative 2: Separate USB-Only Build (Rejected)

- Create two builds: one for disc, one for USB
- **Pros**: Simpler, less memory usage
- **Cons**: User must choose, can't use both, duplicate maintenance

### Alternative 3: Use Existing d2x-cios in Firmware (SELECTED)

- Leverage d2x-cios modules already in HAI-IOS firmware
- Add thin PowerPC layer to communicate with d2x
- **Pros**: Minimal work, proven solution, no module complexity
- **Cons**: Requires d2x in firmware (likely already done)

---

## Success Criteria

### Phase 0 Success (Verification)
- [ ] `/dev/usb2` device opens successfully
- [ ] `USB_GetMode()` returns valid response
- [ ] Existing Riivolution disc loading still works

### Phase 1 Success (API)
- [ ] PowerPC USB API compiles and links
- [ ] `USB_SetWBFSMode()` doesn't crash
- [ ] Commands reach d2x-cios (verified via logging)

### Phase 2 Success (Hardcoded Test)
- [ ] Hardcoded game test runs without crash
- [ ] `WDVD_ReadDiskId()` returns correct game ID
- [ ] Disc reads come from USB (verify via USB activity light)

### Phase 3 Success (Game Launch)
- [ ] Game boots from USB via hardcoded path
- [ ] Game is playable
- [ ] No crashes or freezes

### Phase 4 Success (Riivolution Integration)
- [ ] Riivolution patches apply to USB-loaded games
- [ ] File replacements work from SD card
- [ ] Memory patches work correctly
- [ ] Both disc and USB loading work in same build

### Final Success (Production Ready)
- [ ] USB game browser UI functional
- [ ] Multiple games can be selected and launched
- [ ] Error handling and user feedback
- [ ] Stable across multiple USB drives
- [ ] Documentation complete

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| IOS 56 incompatibility | Medium | High | Use ProxiIOS pattern instead of address patching |
| Module memory overflow | Low | High | Use stripios, optimize modules, remove unused features |
| USB drive compatibility | Medium | Medium | Test with multiple drives, add compatibility list |
| Handler chain breaks | Low | High | Extensive testing, add debug logging |
| Riivolution patches fail | Low | High | Test thoroughly, maintain separate builds |

---

## Next Steps

1. **Review and approve this plan**
2. **Set up development branch** (`git checkout -b usb-loading`)
3. **Start with Milestone 1**: Copy d2x modules and update build system
4. **Create stub implementations** for all new modules
5. **Test module loading** before implementing functionality
6. **Iterate through milestones** with testing at each step

---

## References

- [d2x-cios source](file:///home/yaakov/d2x-cios)
- [usbloadergx source](file:///home/yaakov/usbloadergx)
- [HAI-Riivolution CLAUDE.md](file:///home/yaakov/HAI-Riivolution/CLAUDE.md)
- [Riivolution Wiki](https://aerialx.github.io/rvlution.net/wiki/Main_Page/)
- [WiiBrew - d2x cIOS](https://wiibrew.org/wiki/D2x_cIOS)
