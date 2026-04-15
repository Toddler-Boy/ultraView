// num cells per viewport width
const int GRID_SIZE = 20;	// each grid square is one unit

const float MVMT_SPEED_VAR = 0.1;
const float MVMT_SPEED = 0.5;
const float MVMT_AMPL = 0.5;

const float SIZE = 1.5; // approx. radius in px

const float DARKENING = 5.0;

const float BIG = 1.0e9;
#define dot2(a) dot(a,a)

// credit Dave Hoskins (https://www.shadertoy.com/view/XdGfRR)
vec3 hash32(vec2 q)
{
	uvec3 n = uvec3(ivec3(q.xyx)) * uvec3(1597334673U, 3812015801U, 2798796415U);
	n = (n.x ^ n.y ^ n.z) * uvec3(1597334673U, 3812015801U, 2798796415U);
	return vec3(n) * 2.328306437080797e-10;
}
// credit Dave Hoskins (https://www.shadertoy.com/view/4djSRW)
float hash11(float p)
{
	p = fract(p * .1031);
	p *= p + 33.33;
	p *= p + p;
	return fract(p);
}

float ss(float x)
{
	return 3.0 * x * x - 2.0 * x * x * x;
}

float perlin1(float x)
{
	float f = floor(x);
	float c = f + 1.0;

	float v1 = hash11(f);
	float v2 = hash11(c);

	return 2.0 * mix(v1, v2, ss(fract(x))) - 1.0;
}

// Generates a random star with x,y,and z coordiates
// It will fall in the grid square specified by id
vec3 star ( ivec2 id )
{
	vec3 raw = hash32(vec2(id*2));

	float seed0 = hash11(raw.z);
	float seed1 = hash11(seed0);
	float seed2 = hash11(seed1);

	float speed = MVMT_SPEED + MVMT_SPEED_VAR * (2.0 * seed2 - 1.0);

	raw.x += 0.5 * MVMT_AMPL * perlin1(speed * iTime + 2.0*PI*seed0) / (1.0 + raw.z);

	raw.y += 0.5 * MVMT_AMPL * perlin1(speed * iTime + 2.0*PI*seed1) / (1.0 + raw.z);

	// Fit it in the bottom left cell
	raw.x = raw.x / float(GRID_SIZE);
	raw.y = raw.y / float(GRID_SIZE);

	// ... then move it out
	raw.x += float(id.x) / float(GRID_SIZE);
	raw.y += float(id.y) / float(GRID_SIZE);

	return raw;
}

// The code should explain it
void star_min(inout vec3 closest_star, inout float closest_dist, in vec3 star, in vec2 uv)
{
	float dist = dot2(star.xy-uv);

	if (dist < closest_dist)
	{
		closest_star = star;
		closest_dist = dist;
	}
}

// Just rotate around the origin
vec2 rotate(vec2 v, float theta)
{
	float c = cos(theta);
	float s = sin(theta);
	return vec2(c * v.x - s * v.y, s * v.x + c * v.y);
}


uniform float	ovlDust = 1.0;
uniform float	ovlChromaticAberration = 0.5;
uniform float	ovlChromaticSize = 0.3;
uniform vec2	ovlChromaticCenter = vec2 ( 0.5, 0.5 );
uniform float	ovlGrain = 0.2;

//-----------------------------------------------------------------------------

void main ()
{
	// Get texture
	vec4	pix;

	pix.a = texture ( iChannel0, fragCoord ).a;

	if ( pix.a <= 0.001 )
	 	discard;

	// Chromatic aberration
	vec2	texSize = 1.0 / textureSize ( iChannel0, 0 ).xy;
	vec2	aberrated = ovlChromaticAberration * texSize * vec2 ( 1.0, 0.5 );
	pix.rb = texture ( iChannel0, fragCoord - aberrated ).rb;
	pix.g = texture ( iChannel0, fragCoord + aberrated ).g;

	// Demultiply alpha
	pix.rgb /= pix.a;

	// Color grade
	pix.rgb = colorGrade ( pix.rgb, iChannel2, iChannel3 );

	// Dust
	if ( ovlDust > 0.0 && pix.a == 1.0 )
	{
		// Aspect ratio correction
		vec2	uv = fragCoord * vec2 ( 16.0 / 9.0, 1.0 );

		// Where are we?
		ivec2	cell = ivec2 ( floor ( uv * float ( GRID_SIZE ) ) );

		// Default initialization
		vec3	closest_star = vec3 ( 0.0 );
		float	closest_dist = BIG;

		// check our cell and all surrounding cells
		for ( int i = -1; i <= 1; ++i )
		{
			for ( int j = -1; j <= 1; ++j )
			{
				vec3	ts_star = star ( cell + ivec2 ( i, j ) );
				star_min ( closest_star, closest_dist, ts_star, uv );
			}
		}

		float	dotScale = iResolution.x * 1000;

		// Vector pointing from nearest star to pixel
		vec2	rel_uv = uv - closest_star.xy;
		float	col = min ( 1.0, SIZE * SIZE / dot2 ( rel_uv ) / dotScale ) / pow ( 2.0, DARKENING * closest_star.z );

		pix.rgb = clamp ( pix.rgb + col * ovlDust * 0.2, 0.0, 1.0 );
	}

	if ( ovlGrain > 0.0 )
		pix.rgb *= grnGrain ( uvec2 ( fragCoord * iResolution.xy ), ovlGrain * 0.5 );

	// Back to pre-multiplied
	fragColor = vec4 ( pix.rgb * pix.a, pix.a );
}
//-----------------------------------------------------------------------------
