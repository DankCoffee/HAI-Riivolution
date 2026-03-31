#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// d2x-cios IOCTL commands
#define IOCTL_DI_WBFS_SET    0xF4
#define IOCTL_DI_WBFS_GET    0xF5
#define IOCTL_DI_FRAG_SET    0xF9
#define IOCTL_DI_MODE_GET    0xFA
#define IOCTL_DI_SAVE_CONFIG 0xFE

// d2x-cios modes
#define USB_MODE_NONE   0
#define USB_MODE_WBFS   1
#define USB_MODE_FRAG   2

// d2x-cios devices
#define USB_DEVICE_USB  0
#define USB_DEVICE_SD   1

/**
 * Initialize USB support and detect d2x-cios modules.
 * Call this during launcher initialization.
 *
 * @return 0 on success (d2x-cios detected), negative on error or not present
 */
int USB_Init();

/**
 * Check if USB loading is available (d2x-cios detected).
 *
 * @return 1 if USB loading available, 0 if not (disc-only mode)
 */
int USB_IsAvailable();

/**
 * Query current USB loading mode from d2x-cios.
 *
 * @return Current mode (USB_MODE_NONE, USB_MODE_WBFS, USB_MODE_FRAG) or negative on error
 */
int USB_GetMode();

/**
 * Enable WBFS mode for a specific game ID.
 *
 * @param game_id 6-character game ID (e.g., "RSBE01")
 * @param device USB_DEVICE_USB or USB_DEVICE_SD
 * @return 0 on success, negative on error
 */
int USB_SetWBFSMode(const char* game_id, int device);

/**
 * Disable USB loading and return to disc mode.
 *
 * @return 0 on success, negative on error
 */
int USB_DisableWBFS();

/**
 * Get human-readable status string for diagnostics.
 *
 * @return Status string (e.g., "d2x-cios detected", "Disc-only mode")
 */
const char* USB_GetStatusString();

#ifdef __cplusplus
}
#endif
