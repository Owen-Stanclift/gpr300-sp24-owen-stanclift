#version 450

struct LandmassProp
{
	float scale;
};

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 light_view_proj;
uniform LandmassProp landmass;
uniform sampler2D heightmap;

const float strength = 16.0;
const float kernel[9] = float[](
	1.0, 2.0, 1.0,
	2.0, 4.0, 2.0,
	1.0, 2.0, 1.0 
);

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

void main()
{
	float height = 0;
	for(int i = 0; i <9;i++)
	{
		vec2 uv = vTexCoord + offsets[i].xy;
		float local = texture(heightmap, uv).r;
		height += local * (kernel[i]/ strength);
	}

	vec4 world_pos = model * vec4(vPos,1.0);
	world_pos.y += height * landmass.scale;
	gl_Position = light_view_proj * world_pos;
}