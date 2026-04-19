uniform float	ovlDust = 1.0;
uniform float	ovlChromaticAberration = 0.5;
uniform float	ovlChromaticSize = 0.3;
uniform vec2	ovlChromaticCenter = vec2 ( 0.5, 0.5 );
uniform float	ovlGrain = 0.2;

//-----------------------------------------------------------------------------

uniform vec2	ovlShadowOffset = vec2 ( 0.02, 0.03 );
uniform float	ovlShadow = 0.4;
uniform float	ovlShadowBlur = 4.0;

void main ()
{
	vec4	pix = texture2D ( iChannel0, fragCoord );
	vec2	texSize = 1.0 / textureSize ( iChannel0, 0 ).xy;

	// Chromatic aberration
	vec2	aberrated = ovlChromaticAberration * texSize * vec2 ( 1.0, 0.5 );
	pix.rb = texture ( iChannel0, fragCoord - aberrated ).rb;
	pix.g = texture ( iChannel0, fragCoord + aberrated ).g;

	//
	// Add soft inner-shadow
	//
	if ( ovlShadow > 0.0 )
	{
		float	sourceAlpha = pix.a;

		vec2	basePos = fragCoord + ovlShadowOffset;

		float	shadowIntensity = 0.0;
		vec2	off = texSize * 1.5;

		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( -off.x, 0.0 ), ovlShadowBlur ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( off.x, 0.0 ), ovlShadowBlur ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( 0.0, -off.y ), ovlShadowBlur ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( 0.0, off.y ), ovlShadowBlur ).a;

		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( -off.x, 0.0 ), ovlShadowBlur + 1.0 ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( off.x, 0.0 ), ovlShadowBlur + 1.0  ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( 0.0, -off.y ), ovlShadowBlur + 1.0  ).a;
		shadowIntensity += textureLod ( iChannel0, basePos + vec2 ( 0.0, off.y ), ovlShadowBlur + 1.0  ).a;

		shadowIntensity *= 0.125;

		float	innerShadowAlpha = shadowIntensity * ( 1.0 - sourceAlpha );
		vec4	shadowResult = vec4 ( vec3 ( 0.0 ), ovlShadow * innerShadowAlpha );

		// Blend shadow
		pix += shadowResult;
	}

	// Demultiply alpha
	pix.rgb /= pix.a;

	// Color grade
	pix.rgb = colorGrade ( pix.rgb, iChannel1, iChannel2 );

	// Grain
	if ( ovlGrain > 0.0 )
		pix.rgb *= grnGrain ( uvec2 ( fragCoord * iResolution.xy ), ovlGrain * 0.5 );

	// Back to pre-multiplied
	fragColor = vec4 ( pix.rgb * pix.a, pix.a );
}
//-----------------------------------------------------------------------------
