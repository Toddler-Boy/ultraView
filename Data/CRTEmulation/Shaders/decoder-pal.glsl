//
// PAL decoder
// Uses gamma correction
//
void main ()
{
	// Interference
	vec2	uv = decCreateInterference ( fragCoord );

	// Get chroma-reduced color
	vec3	yuv = decGetBlurredSignal ( uv );

	// Add cross-talk
	yuv = decGetCrosstalk ( yuv, uv );

	// Add noise
	yuv *= grnGrain ( uvec2 ( fragCoord * iResolution.xy ), decNoise * decNoise );

	// Apply brightness, constrast, and saturation
	yuv = encApplyBriConSat ( yuv );

	// Convert YUV to RGB
	fragColor = vec4 ( yuv2rgb ( yuv ), 0.0 );
}
//-----------------------------------------------------------------------------
