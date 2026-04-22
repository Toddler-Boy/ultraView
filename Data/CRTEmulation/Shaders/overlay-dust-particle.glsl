#version 410 core

in float vOpacity;
out vec4 fragColor;

void main()
{
//	fragColor = vec4 ( 1.0 );
//	return;

	float	dist = 1.0 - length ( gl_PointCoord - 0.5 );

	// Disc with soft-edge
	dist = smoothstep ( 0.5 - 0.2, 0.5, dist * 0.5 ) ;

	// Bright kernal with glow
//	dist = pow ( min ( dist * 1.1, 1.0 ), 7.0 );

	fragColor = vec4 ( dist / vOpacity );
}
//-----------------------------------------------------------------------------
