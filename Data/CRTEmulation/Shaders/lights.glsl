uniform	float	ovlBloom = 1.0;

//-----------------------------------------------------------------------------

void main ()
{
	vec4	bloom = getMipMapColor ( iChannel0, fragCoord, 1.0 );
	bloom += getMipMapColor ( iChannel0, fragCoord, 2.0 );
	bloom += getMipMapColor ( iChannel0, fragCoord, 3.0 );
	bloom *= ovlBloom;

	fragColor = bloom + texture ( iChannel0, fragCoord );
}
//-----------------------------------------------------------------------------
