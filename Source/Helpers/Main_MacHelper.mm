#if __APPLE__
#include <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

void setWindowProperties ( void* windowHandle )
{
	NSOperatingSystemVersion	tahoeVersion = { .majorVersion = 26, .minorVersion = 0, .patchVersion = 0 };
	bool	isTahoe = [[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:tahoeVersion];

	auto view = (NSView*) windowHandle;
	view.layer.cornerRadius = isTahoe ? 20.0f : 10.0f;
}
#endif