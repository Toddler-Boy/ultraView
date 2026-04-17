uniform float crtDistortion = 0.3;

//-----------------------------------------------------------------------------

vec2 glassDistortion ( vec2 uv, float k1, float xRatio )
{
	vec2	zoom = vec2 ( 1.0 / ( k1 + 1.0 ), 1.0 / ( k1 + 1.0 ) );

	// 0 to 1 -> -1 to +1
	vec2	cuv = ( uv * 2.0 - 1.0 ) * zoom;

	// Barrel distortion
	cuv *= 1.0 + k1 * ( cuv.x * cuv.x + cuv.y * cuv.y );

	// Add unevenness to glass surface
	cuv += sin ( cuv * PI * 2.5 + PI ) * vec2 ( 0.006, 0.012 ) * k1;// * sin ( iTime ) * 50.0;

	cuv.x *= xRatio;

	// -1 to + 1 -> 0 to 1
	cuv = cuv * 0.5 + 0.5;

	return cuv;
}
//-----------------------------------------------------------------------------

uniform float	crtVignette = 0.33;

float vignette ( vec2 uv )
{
	uv *= 1.0 - uv;
	return clamp ( pow ( uv.x * uv.y * 64.0, crtVignette * 0.25 ), 0.0, 1.0 );
}
//-----------------------------------------------------------------------------

vec3 screen ( vec3 base, vec3 blend )
{
	return 1.0 - ( 1.0 - base) * ( 1.0 - blend );
}
//-----------------------------------------------------------------------------

vec3 add ( vec3 base, vec3 blend )
{
	return base + blend;
}
//-----------------------------------------------------------------------------

uniform bool	crtSource = true;
uniform float	crtReflection = 0.25;
uniform float	crtRflCorrection = 1.0;

uniform vec2	crtShadowScale = vec2 ( 1.0, 1.0 );
uniform vec2	crtShadowTranslate = vec2 ( 0.0, 0.0 );
uniform float	crtShadow = 1.0;

uniform vec3	backCol = vec3 ( 0.0, 0.0, 0.0 );

uniform	vec3	camBrightnessContrastSaturation = vec3 ( 1.0, 1.0, 1.0 );
uniform int		crtWebcamFormat = 0;

void main ()
{
	vec2	cuv = curve ( fragCoord, crtCurve );
	vec3	col = texture ( iChannel0, cuv ).rgb;

	// Add vignette
	col *= vignette ( cuv );

	// Ambient occlusion & shadows
	float	shd = texture ( iChannel1, fragCoord * crtShadowScale + crtShadowTranslate ).a;
	col = clamp ( col - vec3 ( crtShadow * shd * 0.2 ), 0.0, 1.0 );

	// Reflection in glass
	const vec3	glassTint = vec3 ( 0.8, 0.9, 1.0 );

	vec3	rfl = vec3 ( 0.0 );

	if ( crtSource )
	{
		// Glass distortion
		vec2	camCoord = glassDistortion ( vec2 ( 1.0 ) - fragCoord, crtDistortion, crtRflCorrection );
		vec3	yuv;

		// Webcam
		if ( crtWebcamFormat == 0 )
		{
			// NV12
			yuv = vec3 ( texture ( iChannel3, camCoord ).r, texture ( iChannel4, camCoord ).rg );
		}
		// else
		// {
		// 	// YUY2
		// 	vec4	pix = texture ( iChannel5, camCoord );
		// 	float	x = fract ( camCoord.x * ( textureSize ( iChannel5, 0 ).x * 2.0 ) );

		// 	yuv = vec3 ( mix ( pix.r, pix.b, x ), pix.ga );
 		// }
		yuv = ( yuv - vec3 ( 1.0, 0.5, 0.5 ) ) * vec3 ( 1.0, 2.0, 2.0 );

		yuv.yz	*= camBrightnessContrastSaturation.z * 0.5;	// Saturation
		yuv.x	*= camBrightnessContrastSaturation.y;		// Contrast
		yuv.x	+= camBrightnessContrastSaturation.x;		// Brightness

		// YUV -> RGB
		rfl = clamp ( yuv2rgb_mat * yuv, 0.0, 1.0 );
	}
	else
	{
		// Glass reflection texture
		rfl = texture ( iChannel2, glassDistortion ( fragCoord, crtDistortion * 0.1, 1.0 ) ).rgb;
	}

	col = screen ( col, ( rfl * rfl * rfl ) * 0.25 * crtReflection * glassTint );
//	col = rfl;

	// Mask out corners, no CRT has 90 degree angles
	float	mask = roundedMask ( cuv * iResolution.xy, iResolution.xy, iResolution.x * 0.03 );

	// Apply mask
	col = mix ( backCol, col, mask );

	fragColor = vec4 ( col, 1.0 );
}
//-----------------------------------------------------------------------------
