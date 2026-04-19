uniform float	u_alpha = 1.0;

//-----------------------------------------------------------------------------

void main()
{
	float	dist = length ( fragCoord - vec2 ( 0.5, 0.5 ) );
	dist = smoothstep ( 0.5, 0.5 - 0.2, dist ) * u_alpha;

	fragColor = vec4 ( dist );
}
//-----------------------------------------------------------------------------
