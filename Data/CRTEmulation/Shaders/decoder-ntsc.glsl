//
// NTSC decoder
//

void main ()
{
	// Interference
	vec2	uv = decCreateInterference ( fragCoord );

	// Get blurred and chroma-reduced color
	vec3	yiq = decGetBlurredSignal ( uv, iChannel0 );

	// Add cross-talk
	yiq = decGetCrosstalk ( yiq, uv, iChannel0 );

	// Add noise
	yiq *= grnGrain ( uvec2 ( fragCoord * iResolution.xy ), decNoise * decNoise );

	// Apply brightness, constrast, and saturation
	yiq = encApplyBriConSat ( yiq );

	// Convert YIQ to RGB
	fragColor = vec4 ( yiq2rgb ( yiq ), 0.0 );
}
//-----------------------------------------------------------------------------
