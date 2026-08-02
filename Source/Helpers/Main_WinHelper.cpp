#if defined (_WIN32) || defined (_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// App policy, deliberately NOT in the shared PlatformHelper: ultraView's GL
// rendering wants the discrete GPU on switchable-graphics machines

extern "C" {
	// NVIDIA: Enables high-performance mode
	_declspec( dllexport ) DWORD NvOptimusEnablement = 0x00000001;

	// AMD: Enables high-performance mode
	_declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
//-----------------------------------------------------------------------------

#endif
