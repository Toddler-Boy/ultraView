//
// This file contains helpers used by shaders
//
#define PI	3.1415926535897932384626433832795
#define	TAU	( 2.0 * PI )

uniform float crtRefreshRate = 50.125;		// 50.125 for PAL, 59.826 for NTSC

//
// Color space conversions
//
const mat3 yuv2rgb_mat = mat3 ( 1.0,		 1.0,		1.0,
								0.0,		-0.39465,	2.03211,
								1.13983,	-0.581,		0.0 );
//-----------------------------------------------------------------------------

vec3 palGamma ( vec3 rgb )
{
	rgb = pow ( rgb, vec3 ( 1.0 / 2.2 ) );	// sRGB
	rgb = pow ( rgb, vec3 ( 2.8 ) );		// PAL

	return rgb;
}
//-----------------------------------------------------------------------------

vec3 yuv2rgb ( vec3 yuv )
{
	// Convert to RGB
	yuv = yuv2rgb_mat * yuv;

	// Apply gamma correction (2.8 to 2.2)
	yuv = palGamma ( yuv );

	return yuv;
}
//-----------------------------------------------------------------------------

const mat3 yiq2rgb_mat = mat3 ( 1.0,	 1.0,	 1.0,
								1.63,	-0.378, -1.089,
								0.317,	-0.466,  1.677 );
//-----------------------------------------------------------------------------

vec3 yiq2rgb ( vec3 yiq )
{
	return yiq2rgb_mat * yiq;
}
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

//
// Encoder uniforms
//
uniform	vec3	encBrightnessContrastSaturation = vec3 ( 0.5, 1.0, 0.5 );

vec3 encApplyBriConSat ( vec3 lumChrChr )
{
	lumChrChr.yz	*= encBrightnessContrastSaturation.z;
	lumChrChr		*= encBrightnessContrastSaturation.y;
	lumChrChr.x		+= encBrightnessContrastSaturation.x;

	return lumChrChr;
}
//-----------------------------------------------------------------------------

//
// Decoder uniforms
//
float decHash12_v2 ( vec2 p )
{
	vec3	p3 = fract ( vec3 ( p.xyx ) * 0.1031 );
	p3 += dot ( p3, p3.yzx + 33.33 );
	return fract ( ( p3.x + p3.y ) * p3.z );
}
//-----------------------------------------------------------------------------

float decRandom_v2_f ( vec2 p, float t )
{
	return decHash12_v2 ( p * 0.152 + t * 1500.0 + 50.0 );
}
//-----------------------------------------------------------------------------

uniform	float	decInterference = 0.0;

float peak ( float x, float xpos, float scale )
{
	return clamp ( ( 1.0 - x ) * scale * log ( 1.0 / abs ( x - xpos ) ), 0.0, 1.0 );
}
//-----------------------------------------------------------------------------

vec2 decCreateInterference ( vec2 uv )
{
	float	interference = pow ( decInterference, 1.3 );

	float	r = decRandom_v2_f ( vec2 ( 0.0, round ( uv.y * iResolution.y ) ), iTime );
	float	extra = smoothstep ( 1.0 - interference, 1.0, r ) * 4.0;

	float	ifx2 = interference * 0.5 * ( r * peak ( uv.y, 0.2, 0.2 * 0.01 ) );
	uv.x += ( interference / iResolution.x ) * r * 0.4 * extra - ifx2;

	return uv;
}
//-----------------------------------------------------------------------------

uniform	float	decSharpening = 0.1;
uniform	float	decLumablur = 0.5;
uniform	float	decChromablur = 1.0;

#define DEC_FILTER_KERNEL 25
const float decLumaFilter[DEC_FILTER_KERNEL] = float[DEC_FILTER_KERNEL](0.0105,0.0134,0.0057,-0.0242,-0.0824,-0.1562,-0.2078,-0.185,-0.0546,0.1626,0.3852,0.5095,0.5163,0.4678,0.2844,0.0515,-0.1308,-0.2082,-0.1891,-0.1206,-0.0511,-0.0065,0.0114,0.0127,0.008);
const float decChromaFilter[DEC_FILTER_KERNEL] = float[DEC_FILTER_KERNEL](0.001,0.001,0.0001,0.0002,-0.0003,0.0062,0.012,-0.0079,0.0978,0.1059,-0.0394,0.2732,0.2941,0.1529,-0.021,0.1347,0.0415,-0.0032,0.0115,0.002,-0.0001,0.0002,0.001,0.001,0.001);

vec3 decGetBlurredSignal ( vec2 uv )
{
	// chroma sub-sampling
	vec4	lumaChromaDelta = vec4 ( 1.0 / textureSize ( iChannel0, 0 ).x ) * vec4 ( decSharpening, decLumablur, decChromablur, decChromablur );

	vec3	lumaChroma = vec3 ( 0.0 );
	for ( int i = 0; i < DEC_FILTER_KERNEL; i++ )
	{
		vec4	lumaChromaOffset = vec4 ( i - DEC_FILTER_KERNEL / 2 ) * lumaChromaDelta;

		float	sumLuma = texture ( iChannel0, uv + vec2 ( lumaChromaOffset.x, 0.0 ) ).x;
		float	sumLuma2 = texture ( iChannel0, uv + vec2 ( lumaChromaOffset.y, 0.0 ) ).x;
		float	sumChroma = texture ( iChannel0, uv + vec2 ( lumaChromaOffset.z, 0.0 ) ).y - 0.5;
		float	sumChroma2 = texture ( iChannel0, uv + vec2 ( lumaChromaOffset.w, 0.0 ) ).z - 0.5;

		lumaChroma.x += sumLuma * decLumaFilter[ i ] + sumLuma2 * decChromaFilter[ i ];
		lumaChroma.y += sumChroma * decChromaFilter[ i ];
		lumaChroma.z += sumChroma2 * decChromaFilter[ i ];
	}

	return lumaChroma * vec3 ( 0.5, 1.0, 1.0 );
}
//-----------------------------------------------------------------------------

uniform	float	decCrosstalk = 0.25;
uniform	float	decSubcarrier = 0.25;

vec3 decGetCrosstalk ( vec3 signal, vec2 uv )
{
	float	chroma_phase = iTime * crtRefreshRate * 0.5 * PI;
	float	mod_phase = chroma_phase + ( uv.x + uv.y * -0.5 ) * ( 0.5 * PI ) * textureSize ( iChannel0, 0 ).y * 2.0;
	float	subCarrier = decSubcarrier * signal.y;
	float	i_mod = cos ( mod_phase );
	float	q_mod = sin ( mod_phase );

	// crosstalk
	signal.x *= decCrosstalk * subCarrier * q_mod + 1.0;
	signal.y *= subCarrier * i_mod + 1.0;
	signal.z *= subCarrier * q_mod + 1.0;

	return signal;
}
//-----------------------------------------------------------------------------

vec3 grnHash3 ( uint n )
{
	// integer hash copied from Hugo Elias
	n = ( n << 13U ) ^ n;
	n = n * (n * n * 15731U + 789221U) + 1376312589U;
	uvec3	k = n * uvec3 ( n, n * 16807U, n * 48271U );
	return vec3 ( k & uvec3 ( 0x7fffffffU ) ) / float ( 0x7fffffff );
}
//-----------------------------------------------------------------------------

uniform	float	decNoise = 0.0;

vec3 grnGrain ( uvec2 uv, float level )
{
	vec3	grain = grnHash3 ( uv.x + uint ( iResolution.x ) * uv.y + ( uint ( iResolution.x ) * uint ( iResolution.y ) ) * uint ( iTime * 30.0 ) );

	return grain * level * 0.8 + ( 1.0 - ( level * 0.4 ) );
}
//-----------------------------------------------------------------------------

uniform float	decJailbars = 1.0;

float getJailbars ( vec2 uv )
{
	float	tri = fract ( uv.x * uv.y + 0.4 );

	tri = smoothstep ( 0.8, 1.0, tri );

	return ( tri * tri ) * ( decJailbars * decJailbars * decJailbars ) * 0.075;
}
//-----------------------------------------------------------------------------

uniform vec2	crtOverscan = vec2 ( 1.0 );

vec2 overscan ( vec2 uv )
{
	// 0 to 1 -> -1 to +1
	vec2	ouv = ( uv - 0.5 ) * 2.0;

	// Scale
	ouv *= crtOverscan;

	// -1 to + 1 -> 0 to 1
	ouv  = ( ouv / 2.0 ) + 0.5;

	return ouv;
}
//-----------------------------------------------------------------------------

uniform float	crtCurve = 0.4;

vec2 curve ( vec2 uv, float level )
{
	// 0 to 1 -> -1 to +1
	vec2	cuv = ( uv - 0.5 ) * 2.0;

	// shape
	vec2 auv = abs ( cuv.yx ) / vec2 ( 5.0, 4.0 );
	cuv *= 1.0 + auv * auv;

	// -1 to + 1 -> 0 to 1
	cuv  = ( cuv / 2.0 ) + 0.5;

	return mix ( uv, cuv, level );
}
//-----------------------------------------------------------------------------

uniform float lutBlend;

vec3 getLutColor ( sampler3D lut, vec3 color )
{
	vec3	lutSize = vec3 ( textureSize ( lut, 0 ) );
	vec3	scale = ( lutSize - 1.0 ) / lutSize;
	vec3	offset = 1.0 / ( 2.0 * lutSize );

	return texture ( lut, scale * color + offset ).rgb;
}
//-----------------------------------------------------------------------------

vec3 colorGrade ( vec3 pix, sampler3D duskLUT, sampler3D nightLUT )
{
	float	dusk = ( 0.5 - abs ( lutBlend - 0.5 ) ) * 3.0;
	float	night = max ( 0.0, ( lutBlend * 2.0 ) - 1.0 );

	pix = mix ( pix, getLutColor ( duskLUT, pix ), dusk );
	pix = mix ( pix, getLutColor ( nightLUT, pix ), night );

	return pix;
}
//-----------------------------------------------------------------------------

vec3 checkerboard ( vec2 uv, float num )
{
	vec2	pos = floor ( uv * num );
	float	mask = mod ( pos.x + mod ( pos.y, 2.0 ), 2.0 );

	return vec3 ( mask );
}
//-----------------------------------------------------------------------------

float udRoundBox ( vec2 p, vec2 b, float radius )
{
	return length ( max ( abs ( p ) - b + radius, 0.0 ) ) - radius;
}
//-----------------------------------------------------------------------------

float roundedMask ( vec2 uv, vec2 res, float radius )
{
	vec2	p = 2.0 * uv - res;
	float	f = udRoundBox ( p, res, radius );

	return smoothstep ( 4.0, 0.0, f );
}
//-----------------------------------------------------------------------------
