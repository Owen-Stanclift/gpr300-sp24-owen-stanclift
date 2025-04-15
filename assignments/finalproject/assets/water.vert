#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out Surface
{
	vec4 clipSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view_proj;
uniform vec3 camera_position;

uniform float tiling;
uniform float waveSpeed;

void main()
{

	//gl_ClipDistance[0] = 5;
	vec4 WorldPos = model * vec4(vPos, 1.0);
	vs_out.clipSpace = view_proj * WorldPos;
	gl_Position = vs_out.clipSpace;
}