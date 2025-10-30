/*
 * Game-specific compatibility patches
 * Automatically applies critical patches for games with known issues
 * Adapted from USBLoaderGX
 */

#include "gamepatches.h"
#include <string.h>
#include <malloc.h>

// Helper to create a memory patch
static RiiMemoryPatch CreateMemPatch(u32 offset, const void* original, const void* value, u32 length)
{
	RiiMemoryPatch patch;
	patch.Offset = offset;
	patch.Length = length;
	patch.Search = false;  // Direct offset patch
	patch.Ocarina = false;
	patch.Align = 1;

	if (original) {
		patch.Original = (u8*)memalign(32, length);
		memcpy(patch.Original, original, length);
	} else {
		patch.Original = NULL;
	}

	patch.Value = (u8*)memalign(32, length);
	memcpy(patch.Value, value, length);

	return patch;
}

// Helper to create a search-and-replace memory patch
static RiiMemoryPatch CreateSearchPatch(const void* original, const void* value, u32 length, u32 align = 4)
{
	RiiMemoryPatch patch;
	patch.Offset = 0;
	patch.Length = length;
	patch.Search = true;  // Search for pattern
	patch.Ocarina = false;
	patch.Align = align;

	patch.Original = (u8*)memalign(32, length);
	memcpy(patch.Original, original, length);

	patch.Value = (u8*)memalign(32, length);
	memcpy(patch.Value, value, length);

	return patch;
}

// Check if a patch already exists in the disc patches
static bool PatchExists(RiiDisc* disc, const char* patchID)
{
	// Check if patch ID already exists in the disc's patch map
	return disc->Patches.find(patchID) != disc->Patches.end();
}

// Check if ANY patch targeting this specific offset exists
static bool OffsetPatchExists(RiiDisc* disc, u32 offset)
{
	for (auto& patchPair : disc->Patches) {
		for (auto& memPatch : patchPair.second.Memory) {
			if (!memPatch.Search && memPatch.Offset == offset) {
				return true;
			}
		}
	}
	return false;
}

// Add patches for New Super Mario Bros Wii
static void AddNSMBPatches(RiiDisc* disc, const char* gameID)
{
	if (PatchExists(disc, "__builtin_nsmb"))
		return;

	RiiPatch& patch = disc->Patches["__builtin_nsmb"];

	if (memcmp(gameID, "SMNE01", 6) == 0) {
		// NTSC-U version
		if (!OffsetPatchExists(disc, 0x801AB610)) {
			u32 orig = 0x9421FFD0, val = 0x4E800020;
			patch.Memory.push_back(CreateMemPatch(0x801AB610, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CED53)) {
			u32 orig = 0xDA000000, val = 0x71000000;
			patch.Memory.push_back(CreateMemPatch(0x801CED53, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CED6B)) {
			u32 orig = 0xDA000000, val = 0x71000000;
			patch.Memory.push_back(CreateMemPatch(0x801CED6B, &orig, &val, 4));
		}
	}
	else if (memcmp(gameID, "SMNP01", 6) == 0) {
		// PAL version
		if (!OffsetPatchExists(disc, 0x801AB750)) {
			u32 orig = 0x9421FFD0, val = 0x4E800020;
			patch.Memory.push_back(CreateMemPatch(0x801AB750, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CEE90)) {
			u32 orig = 0x38A000DA, val = 0x38A00071;
			patch.Memory.push_back(CreateMemPatch(0x801CEE90, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CEEA8)) {
			u32 orig = 0x388000DA, val = 0x38800071;
			patch.Memory.push_back(CreateMemPatch(0x801CEEA8, &orig, &val, 4));
		}
	}
	else if (memcmp(gameID, "SMNJ01", 6) == 0) {
		// NTSC-J version
		if (!OffsetPatchExists(disc, 0x801AB420)) {
			u32 orig = 0x9421FFD0, val = 0x4E800020;
			patch.Memory.push_back(CreateMemPatch(0x801AB420, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CEB63)) {
			u32 orig = 0xDA000000, val = 0x71000000;
			patch.Memory.push_back(CreateMemPatch(0x801CEB63, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x801CEB7B)) {
			u32 orig = 0xDA000000, val = 0x71000000;
			patch.Memory.push_back(CreateMemPatch(0x801CEB7B, &orig, &val, 4));
		}
	}
}

// Add patches for Prince of Persia
static void AddPoPPatches(RiiDisc* disc, const char* gameID)
{
	if (memcmp(gameID, "SPX", 3) != 0 && memcmp(gameID, "RPW", 3) != 0)
		return;

	if (PatchExists(disc, "__builtin_pop"))
		return;

	RiiPatch& patch = disc->Patches["__builtin_pop"];

	// Byte-order fixes
	if (!OffsetPatchExists(disc, 0x807AAC6A)) {
		u32 orig = 0x7A6B6F6A, val = 0x6F6A7A6B;
		patch.Memory.push_back(CreateMemPatch(0x807AAC6A, &orig, &val, 4));
	}
	if (!OffsetPatchExists(disc, 0x807AAC75)) {
		u32 orig = 0x7C7A6939, val = 0x69397C7A;
		patch.Memory.push_back(CreateMemPatch(0x807AAC75, &orig, &val, 4));
	}
	if (!OffsetPatchExists(disc, 0x807AAC82)) {
		u32 orig = 0x7376686B, val = 0x686B7376;
		patch.Memory.push_back(CreateMemPatch(0x807AAC82, &orig, &val, 4));
	}
	if (!OffsetPatchExists(disc, 0x807AAC92)) {
		u32 orig = 0x80717570, val = 0x75708071;
		patch.Memory.push_back(CreateMemPatch(0x807AAC92, &orig, &val, 4));
	}
	if (!OffsetPatchExists(disc, 0x807AAC9D)) {
		u32 orig = 0x82806F3F, val = 0x6F3F8280;
		patch.Memory.push_back(CreateMemPatch(0x807AAC9D, &orig, &val, 4));
	}
}

// Add patches for Resident Evil 4
static void AddRE4Patches(RiiDisc* disc, const char* gameID)
{
	if (PatchExists(disc, "__builtin_re4"))
		return;

	RiiPatch& patch = disc->Patches["__builtin_re4"];

	u32 orig = 0x38600000, val = 0x38600001;  // Generic original value

	if (memcmp(gameID, "RB4E08", 6) == 0 && !OffsetPatchExists(disc, 0x8016B260)) {
		patch.Memory.push_back(CreateMemPatch(0x8016B260, &orig, &val, 4));
	}
	else if (memcmp(gameID, "RB4P08", 6) == 0 && !OffsetPatchExists(disc, 0x8016B094)) {
		patch.Memory.push_back(CreateMemPatch(0x8016B094, &orig, &val, 4));
	}
	else if (memcmp(gameID, "RB4X08", 6) == 0 && !OffsetPatchExists(disc, 0x8016B0C8)) {
		patch.Memory.push_back(CreateMemPatch(0x8016B0C8, &orig, &val, 4));
	}
}

// Add patches for SD card detection issues
static void AddSDCardPatches(RiiDisc* disc, const char* gameID)
{
	if (PatchExists(disc, "__builtin_sdcard"))
		return;

	RiiPatch& patch = disc->Patches["__builtin_sdcard"];

	// Excite Truck - skip SD card check
	if (memcmp(gameID, "REXE01", 6) == 0 && !OffsetPatchExists(disc, 0x800b9e48)) {
		u32 orig = 0x40820000, val = 0x4800014c;  // branch patch
		patch.Memory.push_back(CreateMemPatch(0x800b9e48, &orig, &val, 4));
	}
	else if (memcmp(gameID, "REXP01", 6) == 0 && !OffsetPatchExists(disc, 0x800ba358)) {
		u32 orig = 0x40820000, val = 0x4800014c;
		patch.Memory.push_back(CreateMemPatch(0x800ba358, &orig, &val, 4));
	}
	else if (memcmp(gameID, "REXJ01", 6) == 0 && !OffsetPatchExists(disc, 0x800ba404)) {
		u32 orig = 0x40820000, val = 0x4800014c;
		patch.Memory.push_back(CreateMemPatch(0x800ba404, &orig, &val, 4));
	}
	// Kirby's Return to Dream Land - NOP out SD checks
	else if (memcmp(gameID, "SUKE01", 6) == 0) {
		if (!OffsetPatchExists(disc, 0x8022da10)) {
			u32 orig = 0x40000000, val = 0x60000000;  // nop
			patch.Memory.push_back(CreateMemPatch(0x8022da10, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x8022da48)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022da48, &orig, &val, 4));
		}
	}
	else if (memcmp(gameID, "SUKP01", 6) == 0) {
		if (!OffsetPatchExists(disc, 0x8022e800)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022e800, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x8022e838)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022e838, &orig, &val, 4));
		}
	}
	else if (memcmp(gameID, "SUKJ01", 6) == 0) {
		if (!OffsetPatchExists(disc, 0x8022c66c)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022c66c, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x8022c6a4)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022c6a4, &orig, &val, 4));
		}
	}
	else if (memcmp(gameID, "SUKK01", 6) == 0) {
		if (!OffsetPatchExists(disc, 0x8022dfc4)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022dfc4, &orig, &val, 4));
		}
		if (!OffsetPatchExists(disc, 0x8022dffc)) {
			u32 orig = 0x40000000, val = 0x60000000;
			patch.Memory.push_back(CreateMemPatch(0x8022dffc, &orig, &val, 4));
		}
	}
}

// Add patches for Mario Kart Wii RCE vulnerability
static void AddMKWiiRCEPatches(RiiDisc* disc, const char* gameID)
{
	if (memcmp(gameID, "RMC", 3) != 0)
		return;

	if (PatchExists(disc, "__builtin_mkwii_rce"))
		return;

	RiiPatch& patch = disc->Patches["__builtin_mkwii_rce"];

	// Determine addresses based on region
	u32 patch_addr = 0;
	u32 patched_check_addr = 0;

	switch (gameID[3]) {
		case 'P':
			patched_check_addr = 0x80276054;
			patch_addr = 0x8089a194;
			break;
		case 'E':
			patched_check_addr = 0x80271d14;
			patch_addr = 0x80895ac4;
			break;
		case 'J':
			patched_check_addr = 0x802759f4;
			patch_addr = 0x808992f4;
			break;
		case 'K':
			patched_check_addr = 0x80263E34;
			patch_addr = 0x808885cc;
			break;
		default:
			return;
	}

	// Patch 7 bytes with 0xFF to fix RCE vulnerability
	// We use search mode to avoid patching if already Wiimmfi-patched
	if (!OffsetPatchExists(disc, patch_addr)) {
		// Create 7-byte patch (aligned to 8 bytes for safety)
		u8 orig[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // Will match unpatched game
		u8 val[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};
		patch.Memory.push_back(CreateMemPatch(patch_addr, orig, val, 7));
	}
}

// Main function to add all built-in compatibility patches
void AddBuiltinPatches(RiiDisc* disc)
{
	// Get game ID from memory (first 6 bytes at 0x80000000)
	const char* gameID = (const char*)0x80000000;

	// Only add patches after the game ID is loaded
	if (!gameID[0])
		return;

	// Add game-specific patches
	AddNSMBPatches(disc, gameID);
	AddPoPPatches(disc, gameID);
	AddRE4Patches(disc, gameID);
	AddSDCardPatches(disc, gameID);
	AddMKWiiRCEPatches(disc, gameID);

	// NOTE: Kirby's extensive patch (1392 patches per region) is provided as a
	// separate Riivolution XML file (kirby_usb.xml) that users can place in their
	// /riivolution/ folder. This allows users to control and customize the patch.

	// Note: Built-in patches are added to disc->Patches but NOT registered
	// in any section/option/choice. This means they won't appear in the menu
	// and won't be selectable by users. However, RVL_PatchMemory() only applies
	// patches that are referenced by enabled options.
	//
	// To make these patches actually apply, we need to ensure they're always active.
	// The cleanest approach: Add them to an always-enabled hidden section.

	// Collect all patch IDs that were actually added
	std::vector<std::string> patchIDs;
	if (disc->Patches.find("__builtin_nsmb") != disc->Patches.end() && !disc->Patches["__builtin_nsmb"].Memory.empty())
		patchIDs.push_back("__builtin_nsmb");
	if (disc->Patches.find("__builtin_pop") != disc->Patches.end() && !disc->Patches["__builtin_pop"].Memory.empty())
		patchIDs.push_back("__builtin_pop");
	if (disc->Patches.find("__builtin_re4") != disc->Patches.end() && !disc->Patches["__builtin_re4"].Memory.empty())
		patchIDs.push_back("__builtin_re4");
	if (disc->Patches.find("__builtin_sdcard") != disc->Patches.end() && !disc->Patches["__builtin_sdcard"].Memory.empty())
		patchIDs.push_back("__builtin_sdcard");
	if (disc->Patches.find("__builtin_mkwii_rce") != disc->Patches.end() && !disc->Patches["__builtin_mkwii_rce"].Memory.empty())
		patchIDs.push_back("__builtin_mkwii_rce");

	if (patchIDs.empty())
		return;

	// Create an always-enabled hidden section that applies these patches automatically
	// Users never see this in the menu, but the patches are always active
	RiiSection section;
	section.Name = "";  // Empty name = hidden from menu
	section.ID = "__builtin_auto";

	RiiOption option;
	option.Name = "";  // Empty name = hidden
	option.ID = "__builtin_auto_opt";
	option.Default = 1;  // Always enabled

	RiiChoice choice;
	choice.Name = "Automatic";
	choice.ID = "__builtin_auto_enabled";

	// Add all non-conflicting patch IDs
	for (const auto& patchID : patchIDs) {
		RiiChoice::Patch p;
		p.ID = patchID;
		choice.Patches.push_back(p);
	}

	option.Choices.push_back(choice);
	section.Options.push_back(option);
	disc->Sections.push_back(section);
}
