//
// NTSC decoder
//

void main ()
{
	// Get YIQ color
	vec3	yiq = texture ( iChannel0, fragCoord ).rgb;

	// Textures can't store negative values, we have to transpose
	yiq.yz -= 0.5;

	// Apply brightness, constrast, and saturation
	yiq = encApplyBriConSat ( yiq );

	// Convert YIQ to RGB
	fragColor = vec4 ( yiq2rgb ( yiq ), 0.0 );
}
//-----------------------------------------------------------------------------
