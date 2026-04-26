uniform float	decJailbars = 1.0;

float getJailbars ( vec2 uv )
{
	float	tri = fract ( uv.x * uv.y + 0.4 );

	tri = smoothstep ( 0.8, 1.0, tri );

	return ( tri * tri ) * ( decJailbars * decJailbars * decJailbars ) * 0.075;
}
//-----------------------------------------------------------------------------
