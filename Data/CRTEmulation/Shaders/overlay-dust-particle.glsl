uniform float	u_alpha = 1.0;

//-----------------------------------------------------------------------------

void main()
{
	float	dist = 1.0 - length ( fragCoord - 0.5 );

	// Disc with soft-edge
	dist = smoothstep ( 0.5 - 0.2, 0.5, dist * 0.5 ) ;

	// Bright kernal with glow
//	dist = pow ( min ( dist * 1.1, 1.0 ), 7.0 );

	fragColor = vec4 ( dist / pow ( 2.0, 3.0 * u_alpha ) );
}
//-----------------------------------------------------------------------------
