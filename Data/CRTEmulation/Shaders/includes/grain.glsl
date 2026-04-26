//-----------------------------------------------------------------------------

vec3 grnHash3 ( uint n )
{
	// integer hash copied from Hugo Elias
	n = ( n << 13U ) ^ n;
	n = n * (n * n * 15731U + 789221U) + 1376312589U;
	uvec3	k = n * uvec3 ( n, n * 16807U, n * 48271U );
	return vec3 ( k & uvec3 ( 0x7fffffffU ) ) / float ( 0x7fffffff );
}
//-----------------------------------------------------------------------------

uniform	float	decNoise = 0.0;

vec3 grnGrain ( uvec2 uv, float level )
{
	vec3	grain = grnHash3 ( uv.x + uint ( iResolution.x ) * uv.y + ( uint ( iResolution.x ) * uint ( iResolution.y ) ) * uint ( iTime * 30.0 ) );

	return grain * level * 0.8 + ( 1.0 - ( level * 0.4 ) );
}
//-----------------------------------------------------------------------------
