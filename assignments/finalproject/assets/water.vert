#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out Surface
{
	vec2 TexCoord;
	vec3 toCamera;
}vs_out;

uniform mat4 model;
uniform mat4 view_proj;
uniform vec3 camera_position;

void main()
{
	vs_out.TexCoord = vTexCoord;

	vec4 WorldPos = model * vec4(vPos,1.0);
	vs_out.toCamera = camera_position - WorldPos.xyz;
	gl_Position = view_proj * WorldPos;
}