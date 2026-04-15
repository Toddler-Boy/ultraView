//
// PAL encoder
// Uses Hanover-bar suppression
//

vec3 getYUV_PAL ( vec2 uv )
{
	uint	index = texture ( iChannel0, uv ).r;
	float	x = 1.0 / textureSize ( iChannel1, 0 ).x;
	uint	row = ( ( uint ( ( uv.y + 1 ) * textureSize ( iChannel0, 0 ).y ) ) & 1u ) ^ 1u;

	return texture ( iChannel1, vec2 ( index * x, row * 0.5 ) ).rgb;
}
//-----------------------------------------------------------------------------

void main ()
{
	// Invert Y source
	vec2	uv = vec2 ( fragCoord.x, 1.0 - fragCoord.y );

	// Convert index to YUV
	vec3	yuv = getYUV_PAL ( uv );

	// Add jailbars
	yuv.r += getJailbars ( vec2 ( fragCoord.x / 8.0, iResolution.x ) );

	// Blend UV with previous lines' UV (Hanover-bar suppression)
	yuv.gb = ( yuv.gb + getYUV_PAL ( uv - vec2 ( 0.0, 1.0 / textureSize ( iChannel0, 0 ).y ) ).gb ) * 0.5;

	// Textures can't store negative values, we have to transpose
	yuv.yz += 0.5;

	fragColor = vec4 ( yuv, 0.0 );
}
//-----------------------------------------------------------------------------
