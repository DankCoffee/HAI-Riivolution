#pragma once

bool PressA();
void PressHome();
void CheckShutdown();
void Initialise();

extern "C" {
	void Init_DebugConsole(const char *ip_str, int port);
	void Init_DebugConsole_Shutdown();
	int DebugConsole_GetStatus();
}

#define DEBUG_NET 1
#define DEBUG_PORT 51016
#define DEBUG_IPADDRESS "192.168.1.26"

#ifdef DEBUG_NET
#define Init_DebugConsole() Init_DebugConsole(DEBUG_IPADDRESS, DEBUG_PORT)
#else
#define Init_DebugConsole() Init_DebugConsole(NULL, 0)
#endif
