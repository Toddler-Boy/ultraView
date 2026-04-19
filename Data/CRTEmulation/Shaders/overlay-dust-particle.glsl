uniform float	u_alpha = 1.0;

//-----------------------------------------------------------------------------

void main()
{
	float	dist = length ( fragCoord - vec2 ( 0.5, 0.5 ) );
	float	alpha = u_alpha * u_alpha;
	dist = smoothstep ( 0.5, 0.5 - 0.2, dist ) * alpha * 0.9 + 0.1;

	fragColor = vec4 ( dist );
}
//-----------------------------------------------------------------------------
