//-----------------------------------------------------------------------------

#define	POW2(x)	(x * x)

float gaussian ( vec2 i, float sigma )
{
	return 1.0 / ( 2.0 * PI * POW2 ( sigma ) ) * exp ( - ( ( POW2 ( i.x ) + POW2 ( i.y ) ) / ( 2.0 * POW2 ( sigma ) ) ) );
}
//-----------------------------------------------------------------------------

vec3 gausBlur ( vec2 uv, int samples )
{
//	return texture ( iChannel0, uv ).rgb;

	const	vec2	scale = vec2 ( 3.0 / 1080.0, 3.0 / 813.0 );

	float	sigma = float ( samples );
	vec3	col = vec3 ( 0.0 );
	vec3	clipper;
	float	accum = 0.0;
	float	weight;
	vec2	offset;

	for ( int x = -samples / 2; x < samples / 2; x++ )
	{
		for ( int y = -samples / 2; y < samples / 2; y++ )
		{
			offset = vec2 ( x, y );
			weight = gaussian ( offset, sigma );
			clipper = texture ( iChannel0, uv + scale * offset ).rgb;

			// Remove ambient color
			clipper = clamp ( clipper - 0.2, 0.0, 1.0 );
			clipper *= 1.0 / 0.8;

			col += clipper * weight;
			accum += weight;
		}
	}

	return col / accum;
}
//-----------------------------------------------------------------------------

uniform float rflLevel;

uniform vec2	rflZoom1 = vec2 ( 1.0 );
uniform vec2	rflShift1 = vec2 ( 0.0 );
uniform int		rflRadius1 = 2;
uniform float	rflAmount1 = 0.5;

uniform vec2	rflZoom2 = vec2 ( 1.0 );
uniform vec2	rflShift2 = vec2 ( 0.0 );
uniform int		rflRadius2 = 20;
uniform float	rflAmount2 = 0.5;

uniform float	ovlGrain = 0.2;

void main ()
{
	// iChannel0 = CRT texture as blur-source
	// iChannel1 = Bezel texture to blend into
	// iChannel2 = 3D LUT for color grading (day to dusk)
	// iChannel3 = 3D LUT for color grading (dusk to night)
	vec4	mask = texture ( iChannel1, fragCoord );

	if ( mask.a < ( 1.0 / 255.0 ) )
		discard;

	// Unmultiply alpha
	mask.rgb /= mask.a;

	// Color grade
	mask.rgb = colorGrade ( mask.rgb, iChannel2, iChannel3 );

	// Reflection
	vec2	uv1 = curve ( fragCoord, crtCurve );
	uv1 -= 0.5;
	vec2	uv2 = uv1;

	// Get first bezel
	uv1 *= rflZoom1;
	uv1 += rflShift1;
	uv1 += 0.5;
	vec3	col = gausBlur ( uv1, rflRadius1 ) * rflAmount1 * rflLevel;

	// Get second bezel
	uv2 *= rflZoom2;
	uv2 += rflShift2;
	uv2 += 0.5;
	col = max ( col, gausBlur ( uv2, rflRadius2 ) * rflAmount2 * rflLevel );

	col += mask.rgb;
	col *= grnGrain ( uvec2 ( fragCoord * iResolution.xy ), ovlGrain );

	fragColor = vec4 ( col * mask.a, mask.a );
}
//-----------------------------------------------------------------------------
