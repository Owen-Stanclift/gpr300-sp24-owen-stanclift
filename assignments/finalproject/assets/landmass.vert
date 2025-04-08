#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 model;
uniform mat4 view_proj;
uniform sampler2D heightmap;

struct LandmassProp
{
	float scale;
};
uniform LandmassProp landmass;

out Surface
{
	vec2 TexCoord;
}vs_out;

//Calculations
const float offset = 1.0 / 2048.0;
const vec2 offsets[9] = vec2[]( 
	vec2(-offset,  offset), // top-left
	vec2( 0.0,     offset), // top-center
	vec2( offset,  offset), // top-right
	vec2(-offset,  0.0),    // center-left
	vec2( 0.0,     0.0),    // center-center
	vec2( offset,  0.0),    // center-right
	vec2(-offset, -offset), // bottom-left
	vec2( 0.0,    -offset), // bottom-center
	vec2( offset, -offset)  // bottom-right    
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
	float height = 0.0;
	for(int i = 0; i <9;i++)
	{
		float local = texture(heightmap,vTexCoord + offsets[i]).r;
		height += local * (kernel[i]/ strength);
	}

	vec3 pos = vPos;
	pos.y += height * landmass.scale;

	gl_Position = view_proj * model * vec4(pos, 1.0);
}