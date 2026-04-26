#include "includes/mathDefines.glsl"
#include "includes/overscan.glsl"

uniform float	crtBleed;
uniform vec2	crtRedOffset = vec2 ( 0.0, 0.0 );
uniform vec2	crtGreenOffset = vec2 ( 0.0, 0.0 );
uniform vec2	crtBlueOffset = vec2 ( 0.0, 0.0 );
uniform float	crtHoffset;
uniform float	crtGlow;
uniform float	crtAmbient = 0.5;
uniform float	crtNoise;
uniform float	crtRefreshRate = 50.125;		// 50.125 for PAL, 59.826 for NTSC

//-----------------------------------------------------------------------------

vec3 srgbToLinear ( vec3 col )
{
	return mix ( col / 12.92, pow ( ( col + 0.055 ) / 1.055, vec3 ( 2.4 ) ), step ( 0.04045, col ) );
}
//-----------------------------------------------------------------------------

vec3 linearToSrgb ( vec3 col )
{
	return mix ( col * 12.92, 1.055 * pow ( col, vec3 ( 1.0 / 2.4 ) ) - 0.055, step ( 0.0031308, col ) );
}
//-----------------------------------------------------------------------------

vec3 bleed ( vec2 uv )
{
	float	x = sin ( 0.1 * iTime + uv.y * 13.0 ) * sin ( 0.23 * iTime + uv.y * 19.0 ) * sin ( 0.3 + 0.11 * iTime + uv.y * 23.0 ) * 0.0012;
	float	o = sin ( fragCoord.y / 2.0 ) / 500.0;
	vec2	xOff = vec2 ( ( x + o * 0.5 ) * crtHoffset, 0.0 );
	vec2	str = vec2 ( -0.005, 0.005 );

	vec3	col = vec3	(	texture ( iChannel0, uv + crtRedOffset * crtBleed * str + xOff ).r,
							texture ( iChannel0, uv + crtGreenOffset * crtBleed * str + xOff ).g,
							texture ( iChannel0, uv + crtBlueOffset * crtBleed * str + xOff ).b );

	return col;
}
//-----------------------------------------------------------------------------

vec3 ambient ( vec3 col )
{
	vec3	ambientCol = mix ( vec3 ( 1.0, 0.95, 1.05 ), vec3 ( 1.0, 0.9, 1.15 ), crtAmbient );
	return max ( col, crtAmbient * 0.16 * ambientCol );
}
//-----------------------------------------------------------------------------

vec3 blurSample ( vec2 uv )
{
	vec3	col = bleed ( uv );

	return col * col * col;
}
//-----------------------------------------------------------------------------

vec3 bloomBlur ( vec2 uv, float offs )
{
	vec4	xoffs = offs * vec4 ( -2.0, -1.0, 1.0, 2.0 ) / ( textureSize ( iChannel0, 0 ).x * 2.0 );
	vec4	yoffs = offs * vec4 ( -2.0, -1.0, 1.0, 2.0 ) / ( textureSize ( iChannel0, 0 ).y * 2.0 );

	vec3	color;

	color  = blurSample ( uv + vec2 ( xoffs.x, yoffs.x ) ) * 0.00366;
	color += blurSample ( uv + vec2 ( xoffs.y, yoffs.x ) ) * 0.01465;
	color += blurSample ( uv + vec2 (     0.0, yoffs.x ) ) * 0.02564;
	color += blurSample ( uv + vec2 ( xoffs.z, yoffs.x ) ) * 0.01465;
	color += blurSample ( uv + vec2 ( xoffs.w, yoffs.x ) ) * 0.00366;

 	color += blurSample ( uv + vec2 ( xoffs.x, yoffs.y ) ) * 0.01465;
	color += blurSample ( uv + vec2 ( xoffs.y, yoffs.y ) ) * 0.05861;
	color += blurSample ( uv + vec2 (     0.0, yoffs.y ) ) * 0.09524;
	color += blurSample ( uv + vec2 ( xoffs.z, yoffs.y ) ) * 0.05861;
	color += blurSample ( uv + vec2 ( xoffs.w, yoffs.y ) ) * 0.01465;

 	color += blurSample ( uv + vec2 ( xoffs.x, 0.0		) ) * 0.02564;
	color += blurSample ( uv + vec2 ( xoffs.y, 0.0		) ) * 0.09524;
	color += blurSample ( uv + vec2 (     0.0, 0.0		) ) * 0.15018;
	color += blurSample ( uv + vec2 ( xoffs.z, 0.0		) ) * 0.09524;
	color += blurSample ( uv + vec2 ( xoffs.w, 0.0		) ) * 0.02564;

 	color += blurSample ( uv + vec2 ( xoffs.x, yoffs.z ) ) * 0.01465;
	color += blurSample ( uv + vec2 ( xoffs.y, yoffs.z ) ) * 0.05861;
	color += blurSample ( uv + vec2 (     0.0, yoffs.z ) ) * 0.09524;
	color += blurSample ( uv + vec2 ( xoffs.z, yoffs.z ) ) * 0.05861;
	color += blurSample ( uv + vec2 ( xoffs.w, yoffs.z ) ) * 0.01465;

 	color += blurSample ( uv + vec2 ( xoffs.x, yoffs.w ) ) * 0.00366;
	color += blurSample ( uv + vec2 ( xoffs.y, yoffs.w ) ) * 0.01465;
	color += blurSample ( uv + vec2 (     0.0, yoffs.w ) ) * 0.02564;
	color += blurSample ( uv + vec2 ( xoffs.z, yoffs.w ) ) * 0.01465;
	color += blurSample ( uv + vec2 ( xoffs.w, yoffs.w ) ) * 0.00366;

	return color;
}
//-----------------------------------------------------------------------------

vec3 bloom ( vec3 col, vec2 uv )
{
	if ( crtGlow < 0.01 )
		return col;

	vec3	blCol;

	blCol  = bloomBlur ( uv, 1.0 );
	blCol += bloomBlur ( uv, 1.5 );
	blCol += bloomBlur ( uv, 2.0 );
	blCol += bloomBlur ( uv, 2.5 );

	return col * ( 1.0 - ( crtGlow * crtGlow ) * 0.1 ) + blCol * 0.125 * ( crtGlow * crtGlow );
}
//-----------------------------------------------------------------------------

float noiseRand ( vec2 co )
{
	return fract ( sin ( dot ( co.xy, vec2 ( 12.9898, 78.233 ) ) ) * 43758.5453 );
}
//-----------------------------------------------------------------------------

vec3 noise ( vec3 col, vec2 uv )
{
	vec2	seed = uv * iResolution.xy;
	vec3	nse = vec3 ( noiseRand ( seed + iTime ), noiseRand ( seed + iTime * 2.0 ), noiseRand ( seed + iTime * 3.0 ) );
	return col + crtNoise * ( nse - 0.5 ) * 0.33;
}
//-----------------------------------------------------------------------------

uniform float	crtScanlines = 0.5;
uniform float	crtMask = 0.5;
uniform float	crtMaskScale = 0.75;
uniform vec3	crtMaskTint = vec3 ( 5.0 );

vec3 scanlines ( vec3 col, float y )
{
	float	luma = dot ( col, vec3 ( 0.299, 0.587, 0.114 ) );
	float	dark = abs ( sin ( PI * y ) ) * PI * 0.5;
	float	line = mix ( dark, 0.85, luma * luma * luma );

	return	col * mix ( 1.0, line, crtScanlines );
}
//-----------------------------------------------------------------------------

vec3 shadowMask ( vec3 col )
{
	vec3	mask = texture ( iChannel1, fragCoord * textureSize ( iChannel1, 0 ) * crtMaskScale ).rgb;
	mask *= crtMaskTint;

	mask = mix ( vec3 ( 1.0 ), mask, crtMask );

	return mask * col;
}
//-----------------------------------------------------------------------------

uniform float	u_deltaTime = 0.016;
uniform	vec3	u_decayFactor = vec3 ( 18.0, 10.0, 20.0 );
uniform float	u_phosphorFlicker = 0.0;

vec3 phosphorDecay ( vec3 col )
{
	col *= abs ( sin ( iTime * crtRefreshRate * PI + fragCoord.y * PI ) * u_phosphorFlicker + ( 1.0 - u_phosphorFlicker ) );

	vec3	prev = texture ( iChannel2, fragCoord ).rgb * u_decayFactor;

	return max ( col, prev );
}
//-----------------------------------------------------------------------------

void main ()
{
	vec3	col;
	vec2	uv = overscan ( fragCoord );

	// CRT-style post FX
	col = bleed ( uv );
	col = srgbToLinear ( col );
	col = scanlines ( col, fragCoord.y * textureSize ( iChannel0, 0 ).y );
	col = shadowMask ( col );
	col = bloom ( col, uv );
	col = linearToSrgb ( col );
	col = ambient ( col );
	col = phosphorDecay ( col );

	fragColor = vec4 ( col, 1.0 );
}
//-----------------------------------------------------------------------------
