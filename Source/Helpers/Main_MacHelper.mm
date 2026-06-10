#if __APPLE__
#include <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

void setWindowProperties ( void* windowHandle, unsigned int titleColor )
{
	NSOperatingSystemVersion	tahoeVersion = { .majorVersion = 26, .minorVersion = 0, .patchVersion = 0 };
	bool	isTahoe = [[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:tahoeVersion];

	auto view = (NSView*) windowHandle;
	view.layer.cornerRadius = isTahoe ? 20.0f : 10.0f;

	NSWindow* window = [view window];
	if ( window )
	{
		CGFloat r = ( ( titleColor >> 16 ) & 0xFF ) / 255.0;
		CGFloat g = ( ( titleColor >> 8  ) & 0xFF ) / 255.0;
		CGFloat b = ( ( titleColor       ) & 0xFF ) / 255.0;

		window.titlebarAppearsTransparent = YES;
		window.backgroundColor = [NSColor colorWithRed:r green:g blue:b alpha:1.0];
	}
}
#endif
