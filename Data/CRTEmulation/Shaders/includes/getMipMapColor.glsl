//-----------------------------------------------------------------------------

vec4 getMipMapColor ( sampler2D tex, vec2 uv, float lod )
{
	float	bias = 0.5 / lod;
	vec2	offset = ( bias / textureSize ( tex, int ( lod ) ) );
	vec4	pix = textureLod ( tex, uv + offset, lod );

	return pix;
}
//-----------------------------------------------------------------------------
