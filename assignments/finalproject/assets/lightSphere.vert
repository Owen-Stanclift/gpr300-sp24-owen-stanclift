#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _CameraViewProjection;
uniform mat4 _Model;

out Surface
{
	vec3 normal;
	vec3 worldPos;
} vs_out;

void main()
{
	vs_out.normal = vNormal;
	vec4 WorldPos = _Model * vec4(vPos,1.0);
	vs_out.worldPos = WorldPos.xyz;
	gl_Position = _CameraViewProjection * WorldPos;
}