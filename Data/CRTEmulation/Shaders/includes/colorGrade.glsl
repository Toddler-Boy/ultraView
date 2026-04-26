//-----------------------------------------------------------------------------

vec3 getLutColor ( sampler3D lut, vec3 color )
{
	vec3	lutSize = vec3 ( textureSize ( lut, 0 ) );
	vec3	scale = ( lutSize - 1.0 ) / lutSize;
	vec3	offset = 1.0 / ( 2.0 * lutSize );

	return texture ( lut, scale * color + offset ).rgb;
}
//-----------------------------------------------------------------------------

uniform float lutBlend;

vec3 colorGrade ( vec3 pix, sampler3D duskLUT, sampler3D nightLUT )
{
	float	dusk = ( 0.5 - abs ( lutBlend - 0.5 ) ) * 3.0;
	float	night = max ( 0.0, ( lutBlend * 2.0 ) - 1.0 );

	pix = mix ( pix, getLutColor ( duskLUT, pix ), dusk );
	pix = mix ( pix, getLutColor ( nightLUT, pix ), night );

	return pix;
}
//-----------------------------------------------------------------------------
