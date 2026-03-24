/*
 * Game-specific compatibility patches
 * Automatically applies critical patches for games with known issues
 * Adapted from USBLoaderGX
 */

#pragma once

#include "riivolution_config.h"

// Add built-in compatibility patches for known problematic games
// These patches are only applied if not already present in user-supplied XML patches
// Called before game is loaded into memory
void AddBuiltinPatches(RiiDisc* disc);
