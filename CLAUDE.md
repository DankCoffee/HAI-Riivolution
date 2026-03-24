# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

HAI-Riivolution is a modified version of Riivolution and RawkSD that runs on Nintendo Wii using HAI-IOS (a custom patched IOS 56). It supports Wii U GamePad (DRC) input and enables game patching, custom content loading, and Rock Band song management.

### IOS Requirements

**For Full Functionality (Disc Patching + USB Loading)**:
- HAI-IOS firmware with d2x-cios modules bundled
- Provides Riivolution patching AND USB game loading

**For USB Loading Only** (Testing/Development):
- Standalone d2x-cios (IOS 56 base)
- Install via d2x-cios-installer to any slot (249, 250, etc.)
- Works independently of HAI-IOS
- **Recommended for testing USB functionality**

**For Disc Patching Only**:
- HAI-IOS without d2x-cios modules
- Original Riivolution functionality
- Automatically falls back to disc-only mode

## Build Commands

**IMPORTANT**: Building MUST be done from the devkitPPC Docker container. Do NOT attempt to build directly from the host system.

### Docker Build (Recommended)

HAI-Riivolution builds successfully with modern devkitPro toolchains.

**Using devkitpro/devkitppc:latest container:**

```bash
cd /path/to/HAI-Riivolution

# Pull the latest devkitPPC container (if not already available)
docker pull devkitpro/devkitppc:latest

# Build all targets
docker run --rm -v "$PWD:/mnt" devkitpro/devkitppc:latest \
  bash -c "cd /mnt/launcher && make -j\$(nproc)"

# Build RawkSD
docker run --rm -v "$PWD:/mnt" devkitpro/devkitppc:latest \
  bash -c "cd /mnt/rawksd && make -j\$(nproc)"

# Clean build
docker run --rm -v "$PWD:/mnt" devkitpro/devkitppc:latest \
  bash -c "cd /mnt/launcher && make clean"
```

**Toolchain versions in container:**
- devkitPPC release 47.1 (gcc 15.1.0)
- devkitARM release 66 (gcc 15.1.0)
- Both PowerPC and ARM code build successfully

### Standard Build (Inside Container)
```bash
# Build all targets (launcher, rawksd, and riifs)
make -j

# Build individual targets
make launcher -j  # Riivolution launcher only
make rawksd -j    # RawkSD only
make riifs        # RiiFS server (PC tool)

# Clean all build artifacts
make clean
```

### Build Environment
- Modern devkitPPC and devkitARM toolchains (gcc 15.1.0)
- All dependencies included in devkitpro/devkitppc:latest container
- No additional package installation required

### Dependencies (If building outside Docker)
- devkitPPC and devkitARM toolchains
- `dkp-pacman -S wii-dev ppc-portlibs wii-portlibs devkitARM`
- `dkp-pacman -S ppc-libogg ppc-libvorbisidec ppc-freetype`
- Python 3.x with pyyaml (for rawksd)
- curl, unzip, zip, and i386 multilibs (for dollz3 tool)

### Debug Output
To enable remote debug output, uncomment `#define DEBUG_NET 1` in `launcher/include/init.h` and update the IP address. Then run on your PC:
```bash
socat TCP4-LISTEN:51016,fork,reuseaddr STDOUT
```

## Pre-Commit Build Verification

This repository includes a pre-commit hook that automatically verifies the launcher builds successfully before allowing commits.

**Location:** `.git/hooks/pre-commit`

The hook will:
1. Check if Docker and devkitpro/devkitppc:latest image are available
2. Build the launcher component using make
3. Block the commit if build fails
4. Clean up build artifacts after verification

**To skip the check** (not recommended):
```bash
git commit --no-verify
```

**First-time setup:**
The pre-commit hook is automatically installed when you clone the repository. It will auto-pull the devkitpro/devkitppc:latest container if needed.

## Architecture Overview

### Three-Layer Architecture

1. **PowerPC Layer (Wii Broadway CPU)**
   - `launcher/` - Main Riivolution application with GUI
   - `rawksd/` - Rock Band song manager and player
   - Both are PPC executables (.dol files) that run on the Wii
   - Use libwiigui for GUI, libxml2 for parsing, libwiidrc for GamePad input

2. **ARM Layer (Starlet Coprocessor - IOS)**
   - `filemodule/` - IOS module for filesystem access (FAT, ISFS, RiiFS)
   - `dipmodule/` - IOS module for disc emulation and USB HID input
   - `megamodule/` - Optional debugger module
   - These are ARM executables (.elf) loaded into IOS memory at runtime
   - Built with devkitARM and linked against libios

3. **PC Tools**
   - `riifs/server-c/` - RiiFS network file server
   - `stripios/` - Tool to strip/minimize IOS modules
   - `dollz3/` - DOL compression/decompression utility

### Module Embedding Process

ARM modules are embedded into PowerPC executables:
```
ARM source → devkitARM → ELF → stripios → binary → bin2o → linked into launcher.dol
```

The launcher loads these embedded modules into IOS at startup via `load_module_code()` in `launcher/source/haxx.cpp`.

### HAI-IOS Integration

- **Target IOS**: IOS 56 (title ID `0x0000000100000038`)
- **Kernel patching**: `launcher/source/haxx.cpp` contains exploit code for IOS signature forgery and module loading
- **ProxiIOS pattern**: IOS modules intercept/proxy device calls (e.g., `/dev/file`, `/dev/di`)
- **Version management**: `launcher/source/init.cpp` handles IOS installation, updates, and downgrades

Key constants in `haxx.cpp`:
- `HAXX_IOS`: Target IOS title ID
- `IOS_DEST`: IOS 56 (0x0000000100000038ULL)
- See line 31 in haxx.cpp

### d2x-cios Integration (USB Loading)

- **Optional**: USB loading requires d2x-cios modules
- **Two deployment options**:
  1. **Standalone d2x-cios**: Install via d2x-cios-installer (IOS 56 base)
  2. **HAI-IOS bundled**: d2x modules built into HAI-IOS firmware
- **Detection**: `launcher/source/usb.cpp` detects d2x-cios at runtime
- **Fallback**: Automatically uses disc-only mode if d2x-cios not present
- **Handler chain**: Riivolution DIP (`/dev/do`) → d2x DIP (`/dev/di`) → Original IOS

### GamePad (DRC) Input

The Wii U GamePad support is implemented in `launcher/lib/libwiidrc/`:

**Initialization pattern:**
```cpp
#include <wiidrc/wiidrc.h>

// In initialization
WiiDRC_Init();

// In main loop
if (WiiDRC_Inited() && WiiDRC_Connected()) {
    WiiDRC_ScanPads();
    u32 buttons = WiiDRC_ButtonsDown();
    s16 lx = WiiDRC_lStickX();
    s16 ly = WiiDRC_lStickY();
    // Handle input...
}
```

Button constants: `WIIDRC_BUTTON_A`, `WIIDRC_BUTTON_B`, `WIIDRC_BUTTON_L`, `WIIDRC_BUTTON_ZR`, etc.

The `dipmodule` also contains USB HID handling (`dipmodule/source/usbhid.cpp`) that maps physical USB gamepads to the DRC interface.

## Key Source Files

### Launcher Core
- `launcher/source/main.cpp` - Application entry point
- `launcher/source/init.cpp` - Initialization, IOS setup, shutdown handling
- `launcher/source/haxx.cpp` - HAI-IOS exploit and module loading
- `launcher/source/launcher.cpp` - Game launching logic, includes anti-002 fix
- `launcher/source/riivolution.cpp` - Riivolution XML patch engine
- `launcher/source/riivolution_config.cpp` - Patch configuration parsing
- `launcher/source/gamepatches.cpp` - Automatic game compatibility patches
- `launcher/source/menu*.cpp` - GUI menu implementations

### IOS Modules
- `filemodule/source/main.cpp` - File module entry point
- `filemodule/source/file_fat.cpp` - FAT filesystem handler
- `filemodule/source/file_isfs.cpp` - ISFS (internal storage) handler
- `filemodule/source/file_riifs.cpp` - RiiFS network filesystem handler
- `dipmodule/source/main.cpp` - DIP module entry point
- `dipmodule/source/dip.cpp` - Disc interface emulation
- `dipmodule/source/usbhid.cpp` - USB HID input handling
- `libios/source/proxiios.cpp` - ProxiIOS framework for module development

### RawkSD Specific
- `rawksd/source/rawksd_main.cpp` - RawkSD entry point
- `rawksd/source/rawksd_menu.cpp` - Song browser UI
- `rawksd/source/rawksd_play.cpp` - Song playback
- `rawksd/kamek/` - Kamek game patching system

## Common Development Patterns

### Adding New Input Handlers

When adding input handling, check all input sources:
```cpp
// Wiimote/Classic Controller
WPAD_ScanPads();
u32 wpad_down = WPAD_ButtonsDown(0);

// GamePad
if (WiiDRC_Inited() && WiiDRC_Connected()) {
    WiiDRC_ScanPads();
    u32 drc_down = WiiDRC_ButtonsDown();
}

// GameCube controller
PAD_ScanPads();
u32 pad_down = PAD_ButtonsDown(0);
```

See examples in `launcher/source/init.cpp` (PressA, PressHome functions).

### IOS Module Development

Modules use the ProxiIOS pattern defined in `libios/`:
1. Implement IPC message handlers
2. Proxy calls to underlying devices
3. Intercept/modify data as needed
4. Return results to PowerPC caller

See `filemodule/source/module.cpp` for reference implementation.

### Riivolution XML Patching

Patch files are XML configs that define:
- File replacements (redirect disc files to SD card)
- Memory patches (modify game code at runtime)
- Folder redirects (mount custom content directories)

Parser is in `launcher/source/riivolution_config.cpp`.

### Automatic Game Compatibility Patches

HAI-Riivolution includes built-in compatibility patches for known problematic games (adapted from USBLoaderGX):

**Implementation**: `launcher/source/gamepatches.cpp`

**Supported Games**:
- New Super Mario Bros Wii (USB compatibility)
- Prince of Persia (byte ordering fixes)
- Resident Evil 4 (USB detection)
- Excite Truck (SD card check bypass)
- Kirby's Return to Dream Land (SD card check bypass - see kirby_usb.xml for full patch)
- Mario Kart Wii (RCE vulnerability patch)

**Behavior**:
- Patches are **always applied automatically** when game ID matches
- **Conflict detection**: Skips patch if user-supplied XML already patches the same memory offset
- **No user interaction**: Patches are invisible and always enabled
- Integration point: `menu_main.cpp:465` (after CombineDiscs, before ParseConfigXMLs)

**Conflict Handling**:
```cpp
// Checks before adding each patch:
1. PatchExists(disc, patchID) - Skip if patch ID already exists
2. OffsetPatchExists(disc, offset) - Skip if ANY patch targets this address
```

User-supplied patches always take precedence over built-in patches.

### Anti-002 Fix

HAI-Riivolution includes the critical anti-002 fix to prevent Error 002 when loading games from USB/SD.

**Implementation**: `launcher/source/launcher.cpp:452-459` (in ApplyBinaryPatches)

**What it does**:
- Searches for the disc authentication check pattern: `{0x2C000000, 0x48000214, 0x3C608000}`
- Changes `0x48000214` (unconditional branch) to `0x40820214` (branch if not equal)
- This allows games to bypass the disc authentication check when loading from USB/SD

**When applied**: During the apploader phase, after each DOL section is loaded into memory

This patch is essential for USB/SD game loading and is applied automatically for all games.

### Kirby's Return to Dream Land Full Patch

For Kirby's Return to Dream Land, an extensive patch (1392 patches per region) is available as separate Riivolution XML files, one per region.

**Files** (75KB each, ~1400 lines):
- `kirby_suke01.xml` - NTSC-U (USA) version
- `kirby_sukp01.xml` - PAL (Europe) version
- `kirby_sukj01.xml` - NTSC-J (Japan) version
- `kirby_sukk01.xml` - Korea version

**Usage**:
1. Copy the appropriate region XML to `/riivolution/` on your SD card
2. HAI-Riivolution will automatically detect and offer it in the patch menu
3. Enable the "Kirby USB/SD Patch" option before launching the game

**What it does**:
- Extensive compatibility patch by crediar
- 1392 memory patches per region
- Enables full USB/SD loading compatibility
- Complements the built-in SD card check bypass

**Why separate**: The patches are region-specific and too large to embed in the binary. Providing them as separate XML files gives users full control and ensures only the correct region's patches are applied.

## Important Build Notes

### Multi-Architecture Compilation
- PowerPC code uses `devkitPPC` (gcc for powerpc-eabi)
- ARM code uses `devkitARM` (gcc for arm-eabi)
- PC tools use native host compiler
- Makefiles handle cross-compilation automatically

### Module Size Constraints
IOS modules must fit in limited ARM memory. Use `stripios` tool to minimize module size by removing unnecessary ELF sections.

### Branch Information
- Main branch: `master`
- Current branch: `USB` (development branch for USB-related features)
- Recent work focuses on IOS 56 compatibility and DIP module improvements

## Commit Message Conventions

All commit messages MUST follow the `[COMPONENT/TYPE]` format. See CONTRIBUTING.md for full details.

**Format:** `[COMPONENT/TYPE] Brief description in useful English`

**Component tags:**
- LAUNCHER, RAWKSD, DIPMODULE, FILEMODULE, MEGAMODULE
- LIBIOS, LIBWIIDRC, RIIFS, BUILD, DOCS, TOOLS

**Type tags:**
- FEAT (new feature), FIX (bug fix), REFACTOR (code refactoring)
- PERF (performance), DOCS (documentation), TEST (tests)
- CHORE (maintenance), HAXX (IOS exploit/kernel changes)

**Examples:**
```
[LAUNCHER/FIX] Fix memory leak in XML parser
[DIPMODULE/FEAT] Add USB keyboard support to HID driver
[LIBWIIDRC/FIX] Correct analog stick deadzone calculation
[BUILD/CHORE] Update dependencies for devkitPPC r42
```

**Important:**
- Do not include AI tool attributions or generation notices
- Focus on technical changes and structural improvements

**Setting up commit template:**
```bash
git config commit.template .gitmessage
```

## Security Context

This codebase contains:
- **IOS exploitation code** (`haxx.cpp`) - Uses signature forgery and cryptographic operations to load custom modules
- **Kernel patching** - Modifies Wii system software (IOS) at runtime
- **DRM circumvention** - Enables running modified game content

This is legitimate homebrew software for:
- Running custom content on owned hardware
- Educational/research purposes
- Game modding and preservation

Do not use this code for piracy or unauthorized distribution of copyrighted content.
