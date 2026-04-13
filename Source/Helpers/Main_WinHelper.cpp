#if defined (_WIN32) || defined (_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//-----------------------------------------------------------------------------

void setWindowProperties ( void* windowHandle, unsigned int titleColor )
{
	if ( auto hDwmApi = LoadLibrary ("dwmapi.dll"); hDwmApi )
	{
		typedef HRESULT ( WINAPI* PFNSETWINDOWATTRIBUTE )( HWND hWnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute );

		if ( auto pfnSetWindowAttribute = reinterpret_cast<PFNSETWINDOWATTRIBUTE>( GetProcAddress ( hDwmApi, "DwmSetWindowAttribute" ) ); pfnSetWindowAttribute )
		{
			enum : DWORD
			{
				DWMWA_BORDER_COLOR = 34,
				DWMWA_CAPTION_COLOR,

				DWMWA_COLOR_NONE = 0xFFFFFFFE,
				DWMWA_COLOR_DEFAULT,
			};

			DWORD	color = ( ( titleColor >> 16 ) & 0xFF ) | ( ( titleColor << 16 ) & 0xFF ) | ( titleColor & 0x00FF00 );
 			pfnSetWindowAttribute ( (HWND)windowHandle, DWMWA_CAPTION_COLOR, &color, sizeof ( color ) );
		}
		FreeLibrary ( hDwmApi );
	}
}
//-----------------------------------------------------------------------------

#endif
