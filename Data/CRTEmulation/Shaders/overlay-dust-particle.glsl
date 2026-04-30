#version 410 core

#ifdef UPDATE

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inVel;

out vec3 outPos;
out vec3 outVel;

uniform float	deltaTime = 0.016666;
uniform float	iTime = 0;

float hash ( vec3 p )
{
	return fract (sin (dot (p, vec3 ( 12.9898, 78.233, 45.164 ) ) ) * 43758.5453 );
}

// Generate a random vector for a specific point in space
vec3 getWindVector ( vec3 p )
{
	// We scale iTime down (0.2) so the "air currents" don't vibrate too fast
	vec3 seed = p + ( iTime * 0.2 );

	float	x = hash ( seed.xyz ) * 2.0 - 1.0;
	float	y = hash ( seed.yzx ) * 2.0 - 1.025; // Slightly more downward bias for dust
	float	z = hash ( seed.zxy ) * 2.0 - 1.0;

	return vec3 ( x, y, z );
}

// Helper to normalize Z into a 0.0 -> 1.0 range
float getZFactor ( float z )
{
	return (z + 1.0) * 0.5;
}

void main ()
{
	const float	jitterStrength = 1.1;
	const float	drag = 0.99;

	vec3	windCurrent = getWindVector ( inPos * 0.5 );

	vec3	vel = inVel + windCurrent * jitterStrength * deltaTime;
	vel *= drag;

	float	zNext = clamp ( inPos.z + vel.z, -1.0, 1.0 );
	float	zFactor = getZFactor ( zNext );

	// Square the zFactor to make the speed difference between front and back more dramatic
	float	speedMultiplier = mix ( 0.00005, 0.003, zFactor * zFactor );

	vec3	pos = inPos + ( vel * speedMultiplier );

	pos.xy = mod ( pos.xy + 1.0, 2.0 ) - 1.0;
	pos.z = clamp ( pos.z, -1.0, 1.0 );

	outPos = pos;
	outVel = vel;
}
//-----------------------------------------------------------------------------
#endif

#ifdef VERTEX

layout (location = 0) in vec3 aInstance;

out float vOpacity;

void main ()
{
	vOpacity = pow ( 2.0, 2.5 * ( 1.0 - ( aInstance.z + 1.0 ) * 0.5 ) );

	gl_Position = vec4 ( aInstance.xy, 0.0, 1.0 );
	gl_PointSize = 5.0;
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
