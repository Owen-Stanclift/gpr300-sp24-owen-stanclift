#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view_proj;
uniform mat4 light_view_proj;

uniform sampler2D heightmap;
uniform vec4 clipping_plane;

struct LandmassProp
{
	float scale;
};
uniform LandmassProp landmass;

out Surface
{
	vec2 TexCoord;
	vec4 LightSpacePosition;
	vec3 WorldPosition;
}vs_out;

//Calculations

vec3 light;
const float offset = 1.0 / 2049.0;
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

void main()
{
	vs_out.TexCoord = vTexCoord;
	float height = 0;
	for(int i = 0; i <9;i++)
	{
		vec2 hPos = vTexCoord + offsets[i].xy;
		float local = texture(heightmap, hPos).r;
		height += local * (kernel[i]/ strength);
	}

	vec4 WorldPos = model * vec4(vPos, 1.0);
		vs_out.LightSpacePosition = light_view_proj * WorldPos;
	WorldPos.y += height * landmass.scale;
	vs_out.WorldPosition = WorldPos.xyz;


	gl_ClipDistance[0] = dot(WorldPos, clipping_plane);
	gl_Position = view_proj * WorldPos;
}


