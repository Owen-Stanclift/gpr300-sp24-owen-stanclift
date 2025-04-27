#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view_proj;
uniform sampler2D heightmap;
uniform vec4 clipping_plane;
uniform vec3 lightPos;
uniform mat4 lightProj;

struct LandmassProp
{
	float scale;
};
uniform LandmassProp landmass;

out Surface
{
	vec2 TexCoord;
	float inShadow;
	float normalShadow;
	vec4 lightPosition;
	vec3 WorldPosition;
}vs_out;

//Calculations

float getHeight(int x, int z)
{
	float height = texture(heightmap,vec2(x,z)).r;
	return height;
}
vec3 light;
const float offset = 1.0 / 2048.0;
const vec3 offsets[9] = vec3[]( 
	vec3(-offset,  offset,0), // top-left
	vec3( 0.0,     offset,0), // top-center
	vec3( offset,  offset,0), // top-right
	vec3(-offset,  0.0,0),    // center-left
	vec3( 0.0,     0.0,0),    // center-center
	vec3( offset,  0.0,0),    // center-right
	vec3(-offset, -offset,0), // bottom-left
	vec3( 0.0,    -offset,0), // bottom-center
	vec3( offset, -offset,0)  // bottom-right    
);

const float strength = 16.0;
const float kernel[9] = float[](
	1.0, 2.0, 1.0,
	2.0, 4.0, 2.0,
	1.0, 2.0, 1.0 
);
const float steps = 10;
void shadowHeightMap(vec2 texOffset,float height)
{

		vec3 p = vec3(texOffset,height);
		vec3 lightOffset = vec3(light.x,-light.z,-light.y);
		vec3 stepDir = (lightOffset - p)/100;
		for(int i=0; i < steps; i++)
		{
		vs_out.inShadow = 0;
		p += stepDir;
		float h = texture(heightmap,p.xy).r;

		if(h > p.z)
			{
			vs_out.inShadow = 1;
			break;
			}

		if(p.z > 1)
			{
			break;
			}
		}
}

vec3 calculateNormal(vec2 pos)
{
	vec3 off = vec3(1.0,1.0,0.0);
	float hL = dot(pos,-off.xz);
	float hR = dot(pos,off.xz);
	float hD = dot(pos,-off.zy);
	float hU = dot(pos,off.zy);


	vec3 N = vec3(0);
	N.x = hL - hR;
	N.y = hD - hU;
	N.z = 2.0;
	N = normalize(N);
	return N;
};
void main()
{
	light = lightPos * vec3(1,-1,1);
	vs_out.TexCoord = vTexCoord;
	float height = 0.0;
	for(int i = 0; i <9;i++)
	{
		vec2 hPos = vTexCoord + offsets[i].xy;
		float local = texture(heightmap,hPos).r;
		height += local * (kernel[i]/ strength);
		shadowHeightMap(hPos,height);
	}
	

	vec3 lightDir = light - vPos;
	vec3 stepDir = normalize(lightDir);
	vec4 WorldPos = model * vec4(vPos, 1.0);
	WorldPos.y += height * landmass.scale;
	vs_out.lightPosition = lightProj * WorldPos;
	vs_out.WorldPosition = WorldPos.xyz;
	vec3 norm = calculateNormal(vTexCoord);
	vs_out.normalShadow = (dot(norm, -stepDir) + 1)/2;
	gl_ClipDistance[0] = dot(WorldPos, clipping_plane);
	gl_Position = view_proj * WorldPos;
}


