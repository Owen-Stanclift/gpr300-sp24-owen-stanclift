#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out Surface
{
	vec2 TexCord;
	vec4 clipSpace;
	vec3 to_camera;
	vec3 lightVec;
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view_proj;
uniform vec3 camera_position;
uniform vec3 lightPosition;

void main()
{
	//vs_out.TexCord =  vTexCoord;
	//gl_ClipDistance[0] = 5;
	vec4 WorldPos = model * vec4(vPos, 1.0);
	vs_out.TexCord =  vTexCoord;
	vs_out.clipSpace = view_proj * WorldPos;
	vs_out.to_camera = WorldPos.xyz - camera_position;
	vs_out.lightVec =  vec3(lightPosition.x,-lightPosition.y,lightPosition.z) - WorldPos.xyz;
	vs_out.normal = vNormal;
	gl_Position = vs_out.clipSpace;
}