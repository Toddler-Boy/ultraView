//
// YIQ encoder
//

void main ()
{
	// Convert index to YIQ
	uint	index = texture ( iChannel0, vec2 ( fragCoord.x, 1 - fragCoord.y ) ).r;
	float	x = 1.0 / textureSize ( iChannel1, 0 ).x;
	vec3	col = texture ( iChannel1, vec2 ( index * x, 1 ) ).rgb;

	// Textures can't store negative values, we have to transpose
	col.yz += 0.5;

	fragColor = vec4 ( col, 0.0 );
}
//-----------------------------------------------------------------------------
