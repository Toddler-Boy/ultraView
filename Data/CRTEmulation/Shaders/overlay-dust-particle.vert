#version 410 core

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
