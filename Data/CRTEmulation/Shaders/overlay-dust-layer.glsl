uniform float	ovlDust = 1.0;
uniform	float	ovlBloom = 1.0;

//-----------------------------------------------------------------------------

void main ()
{
	float	scaleFactor = clamp ( textureSize ( iChannel0, 0 ).x / 3840.0, 0.3, 1.0 );

	vec4	dust = texture ( iChannel0, fragCoord ) * ovlDust * scaleFactor;
	vec4	mask = getMipMapColor ( iChannel1, ( fragCoord - 0.5 ) * 0.8 + 0.5, 5.5 );

	vec4	bloom = getMipMapColor ( iChannel1, fragCoord, 3.0 );
	bloom += getMipMapColor ( iChannel1, fragCoord, 4.0 );
	bloom += getMipMapColor ( iChannel1, fragCoord, 5.5 );
	bloom += getMipMapColor ( iChannel1, fragCoord, 6.5 );
	bloom += getMipMapColor ( iChannel1, fragCoord, 7.5 );
	bloom *= 0.25;
	bloom.rgb = colorGrade ( bloom.rgb, iChannel2, iChannel3 );
	bloom = bloom * bloom * bloom * ovlBloom;

	fragColor = bloom + ( dust * mix ( 0.2, 1.0, mask.a ) );
}
//-----------------------------------------------------------------------------
