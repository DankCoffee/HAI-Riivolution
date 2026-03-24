#include "usb.h"
#include "wdvd.h"
#include "init.h"
#include <gccore.h>
#include <stdio.h>
#include <string.h>

// Hardcoded test values - change these for your USB setup
#define TEST_GAME_ID "RSBE01"  // Super Smash Bros. Brawl (NTSC-U)
#define TEST_DEVICE  USB_DEVICE_USB  // 0=USB, 1=SD

// Alternative game IDs for testing:
// "RMCE01" - Mario Kart Wii (NTSC-U)
// "SMNE01" - Super Mario Galaxy (NTSC-U)
// "RSPP01" - Wii Sports Resort (NTSC-U)

int USB_TestHardcodedGame()
{
	printf("\n");
	printf("========================================\n");
	printf("  USB Hardcoded Game Test (Phase 1)\n");
	printf("========================================\n");
	printf("Game ID: %s\n", TEST_GAME_ID);
	printf("Device:  %s\n", TEST_DEVICE == USB_DEVICE_USB ? "USB" : "SD");
	printf("\n");

	// Check if USB loading is available
	if (!USB_IsAvailable()) {
		printf("ERROR: USB loading not available!\n");
		printf("d2x-cios was not detected at startup.\n");
		printf("\n");
		printf("To enable USB loading:\n");
		printf("1. Install d2x-cios (IOS 56 base) via d2x-cios-installer\n");
		printf("   OR\n");
		printf("2. Use HAI-IOS firmware with d2x-cios modules\n");
		printf("\n");
		printf("Press A to continue...\n");
		return -1;
	}

	printf("d2x-cios detected - USB loading available\n");
	printf("\n");

	// Step 1: Check current mode
	printf("[1/4] Checking current USB mode...\n");
	int mode = USB_GetMode();
	if (mode < 0) {
		printf("      ERROR: Failed to get mode (ret=%d)\n", mode);
		return mode;
	}
	printf("      Current mode: %d ", mode);
	switch (mode) {
		case USB_MODE_NONE:
			printf("(None - disc mode)\n");
			break;
		case USB_MODE_WBFS:
			printf("(WBFS mode)\n");
			break;
		case USB_MODE_FRAG:
			printf("(Fraglist mode)\n");
			break;
		default:
			printf("(Unknown)\n");
			break;
	}
	printf("\n");

	// Step 2: Enable WBFS mode for test game
	printf("[2/4] Setting WBFS mode for %.6s...\n", TEST_GAME_ID);
	int ret = USB_SetWBFSMode(TEST_GAME_ID, TEST_DEVICE);
	if (ret < 0) {
		printf("      ERROR: Failed to set WBFS mode (ret=%d)\n", ret);
		printf("\n");
		printf("Common causes:\n");
		printf("- USB drive not inserted or not detected\n");
		printf("- Game not found in /wbfs/ directory\n");
		printf("- Incorrect game folder structure\n");
		printf("\n");
		printf("Expected structure:\n");
		printf("  /wbfs/Game Name [%.6s]/%.6s.wbfs\n", TEST_GAME_ID, TEST_GAME_ID);
		printf("\n");
		return ret;
	}
	printf("      WBFS mode enabled successfully\n");
	printf("\n");

	// Step 3: Verify mode changed
	printf("[3/4] Verifying mode change...\n");
	mode = USB_GetMode();
	printf("      New mode: %d ", mode);
	if (mode == USB_MODE_WBFS) {
		printf("(WBFS mode) - CORRECT\n");
	} else {
		printf("- WARNING: Expected mode 1 (WBFS)\n");
	}
	printf("\n");

	// Step 4: Try to read disc ID
	printf("[4/4] Reading disc ID from USB...\n");
	u32 disc_id[8] ATTRIBUTE_ALIGN(32);
	memset(disc_id, 0, sizeof(disc_id));

	ret = WDVD_LowReadDiskId();
	if (ret != 0) {
		printf("      ERROR: Failed to read disc ID (ret=%d)\n", ret);
		printf("\n");
		printf("This usually means:\n");
		printf("- Game file is corrupted or invalid\n");
		printf("- USB drive communication error\n");
		printf("- d2x-cios USB module not working correctly\n");
		printf("\n");
		printf("Cleaning up...\n");
		USB_DisableWBFS();
		return ret;
	}

	// WDVD_LowReadDiskId() writes to 0x80000000, copy it to our buffer
	memcpy(disc_id, (void*)0x80000000, 32);
	printf("      Disc ID read: %.6s\n", (char*)disc_id);
	printf("      Expected:     %.6s\n", TEST_GAME_ID);
	printf("\n");

	// Step 5: Compare disc IDs
	if (memcmp(disc_id, TEST_GAME_ID, 6) != 0) {
		printf("WARNING: Disc ID mismatch!\n");
		printf("The game loaded may not be the expected one.\n");
		printf("\n");
	} else {
		printf("SUCCESS: Disc ID matches!\n");
		printf("\n");
	}

	// Summary
	printf("========================================\n");
	printf("  Test Complete\n");
	printf("========================================\n");
	printf("\n");
	printf("Game %.6s is ready to launch from USB.\n", (char*)disc_id);
	printf("\n");
	printf("Next steps:\n");
	printf("- Use normal Riivolution game launch process\n");
	printf("- Game will boot from USB drive\n");
	printf("- Riivolution patches will apply on top of USB data\n");
	printf("\n");
	printf("To disable USB mode and return to disc:\n");
	printf("- Restart the launcher\n");
	printf("- USB mode resets on application exit\n");
	printf("\n");

	return 0;
}
