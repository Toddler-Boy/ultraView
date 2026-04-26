#include "includes/mathDefines.glsl"
#include "includes/common.glsl"
#include "includes/colorSpaces.glsl"

//-----------------------------------------------------------------------------

//
// PAL decoder
//

void main ()
{
	// Get YUV color
	vec3	yuv = texture ( iChannel0, fragCoord ).rgb;

	// Textures can't store negative values, we have to transpose
	yuv.yz -= 0.5;

	// Apply brightness, constrast, and saturation
	yuv = encApplyBriConSat ( yuv );

	// Convert YUV to RGB
	fragColor = vec4 ( yuv2rgb ( yuv ), 0.0 );
}
//-----------------------------------------------------------------------------
