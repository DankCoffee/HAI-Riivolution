#include "usb.h"
#include <gccore.h>
#include <ogc/ios.h>
#include <stdio.h>
#include <string.h>

// Global state
static int g_usb_available = 0;
static const char* g_usb_status = "USB not initialized";

int USB_Init()
{
	printf("USB: Initializing USB support...\n");
	g_usb_available = 0;
	g_usb_status = "Disc-only mode (d2x-cios not detected)";

	// Step 1: Check for /dev/usb2 device (d2x-cios USB module)
	printf("USB: Checking for /dev/usb2...\n");
	int usb_fd = IOS_Open("/dev/usb2", 0);
	if (usb_fd < 0) {
		printf("USB: /dev/usb2 not found (ret=%d)\n", usb_fd);
		printf("USB: d2x-cios USB module not present in firmware\n");
		printf("USB: Defaulting to disc-only mode\n");
		return -1;
	}
	printf("USB: /dev/usb2 found (fd=%d)\n", usb_fd);
	IOS_Close(usb_fd);

	// Step 2: Check if d2x-cios DIP plugin is active by sending MODE_GET
	printf("USB: Testing d2x-cios DIP plugin via /dev/do...\n");
	int di_fd = IOS_Open("/dev/do", 0);
	if (di_fd < 0) {
		printf("USB: Failed to open /dev/do (ret=%d)\n", di_fd);
		printf("USB: Riivolution DIP module not loaded?\n");
		return -2;
	}

	// Send MODE_GET command to test d2x-cios response
	u32 inbuf[8] ATTRIBUTE_ALIGN(32);
	u32 outbuf[8] ATTRIBUTE_ALIGN(32);
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));

	inbuf[0] = IOCTL_DI_MODE_GET << 24;

	int ret = IOS_Ioctl(di_fd, IOCTL_DI_MODE_GET, inbuf, sizeof(inbuf),
	                    outbuf, sizeof(outbuf));

	IOS_Close(di_fd);

	if (ret < 0) {
		printf("USB: MODE_GET failed (ret=%d)\n", ret);
		printf("USB: d2x-cios DIP plugin not responding\n");
		printf("USB: This is normal if d2x-cios is not in HAI-IOS firmware\n");
		printf("USB: Defaulting to disc-only mode\n");
		return -3;
	}

	// Success! d2x-cios is present and responding
	int mode = outbuf[0];
	printf("USB: d2x-cios DIP plugin detected!\n");
	printf("USB: Current mode: %d\n", mode);
	printf("USB: USB loading is AVAILABLE\n");

	g_usb_available = 1;
	g_usb_status = "d2x-cios detected - USB loading available";

	return 0;
}

int USB_IsAvailable()
{
	return g_usb_available;
}

int USB_GetMode()
{
	if (!g_usb_available) {
		return -1;
	}

	int fd = IOS_Open("/dev/do", 0);
	if (fd < 0) {
		return -2;
	}

	u32 inbuf[8] ATTRIBUTE_ALIGN(32);
	u32 outbuf[8] ATTRIBUTE_ALIGN(32);
	memset(inbuf, 0, sizeof(inbuf));
	memset(outbuf, 0, sizeof(outbuf));

	inbuf[0] = IOCTL_DI_MODE_GET << 24;

	int ret = IOS_Ioctl(fd, IOCTL_DI_MODE_GET, inbuf, sizeof(inbuf),
	                    outbuf, sizeof(outbuf));

	IOS_Close(fd);

	return (ret >= 0) ? (int)outbuf[0] : ret;
}

int USB_SetWBFSMode(const char* game_id, int device)
{
	if (!g_usb_available) {
		printf("USB: USB loading not available\n");
		return -1;
	}

	if (!game_id || strlen(game_id) < 6) {
		printf("USB: Invalid game ID\n");
		return -2;
	}

	int fd = IOS_Open("/dev/do", 0);
	if (fd < 0) {
		printf("USB: Failed to open /dev/do\n");
		return -3;
	}

	u32 inbuf[8] ATTRIBUTE_ALIGN(32);
	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_WBFS_SET << 24;
	inbuf[1] = device;
	memcpy(&inbuf[2], game_id, 6);

	printf("USB: Setting WBFS mode for %.6s on device %d\n", game_id, device);

	int ret = IOS_Ioctl(fd, IOCTL_DI_WBFS_SET, inbuf, sizeof(inbuf), NULL, 0);

	IOS_Close(fd);

	if (ret < 0) {
		printf("USB: WBFS_SET failed (ret=%d)\n", ret);
	} else {
		printf("USB: WBFS mode enabled\n");
	}

	return ret;
}

int USB_DisableWBFS()
{
	if (!g_usb_available) {
		return -1;
	}

	int fd = IOS_Open("/dev/do", 0);
	if (fd < 0) {
		return -2;
	}

	u32 inbuf[8] ATTRIBUTE_ALIGN(32);
	memset(inbuf, 0, sizeof(inbuf));

	inbuf[0] = IOCTL_DI_WBFS_SET << 24;
	inbuf[1] = 0xFF; // Invalid device = disable

	printf("USB: Disabling WBFS mode\n");

	int ret = IOS_Ioctl(fd, IOCTL_DI_WBFS_SET, inbuf, sizeof(inbuf), NULL, 0);

	IOS_Close(fd);

	if (ret < 0) {
		printf("USB: WBFS disable failed (ret=%d)\n", ret);
	} else {
		printf("USB: WBFS mode disabled\n");
	}

	return ret;
}

const char* USB_GetStatusString()
{
	return g_usb_status;
}
