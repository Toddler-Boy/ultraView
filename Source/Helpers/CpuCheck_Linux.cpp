#if defined (__linux__)

#include <cstdio>
#include <cstdlib>
#include <string>

// The binary is built for SSE 4.2. This TU compiles at baseline x86-64
// (CMakeLists) and runs before every other static initializer. Hard-coded
// text: no data source yet. Dialog best effort, stderr always
__attribute__ (( constructor ( 101 ) )) static void checkCpuBaseline ()
{
	// libgcc fills the feature table at the same priority
	__builtin_cpu_init ();

	if ( __builtin_cpu_supports ( "sse4.2" ) )
		return;

	const std::string	message = "ultraView needs a CPU with SSE 4.2 (Intel Nehalem, AMD Bulldozer or newer).";

	std::fprintf ( stderr, "%s\n", message.c_str () );

	if ( std::system ( ( "zenity --error --title=ultraView --text=\"" + message + "\" 2>/dev/null" ).c_str () ) != 0 )
		std::system ( ( "xmessage -center \"" + message + "\" 2>/dev/null" ).c_str () );

	std::_Exit ( 1 );
}

#endif
