#version 410 core

#ifdef VERTEX

layout (location = 0) in vec3 aInstance;
uniform float dustSize = 5.0;

out float vOpacity;

void main ()
{
	vOpacity = pow ( 2.0, 2.5 * ( 1.0 - aInstance.z ) );

	gl_Position = vec4 ( aInstance.xy, 0.0, 1.0 );
	gl_PointSize = dustSize;
}
//-----------------------------------------------------------------------------
#endif


#ifdef FRAGMENT

in float vOpacity;
out vec4 fragColor;

void main ()
{
	float	dist = 1.0 - length ( gl_PointCoord - 0.5 );

	// Disc with soft-edge
	dist = smoothstep ( 0.5 - 0.2, 0.5, dist * 0.5 ) ;

	// Bright kernal with glow
//	dist = pow ( min ( dist * 1.2, 1.0 ), 7.0 );

	fragColor = vec4 ( dist / vOpacity );
}
//-----------------------------------------------------------------------------
#endif
