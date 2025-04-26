#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _CameraViewProjection;
uniform mat4 _Model;

uniform vec4 clipping_plane;


void main()
{
	vec4 WorldPos = _Model * vec4(vPos,1.0);
	gl_ClipDistance[0] = dot(WorldPos, clipping_plane);
	gl_Position = _CameraViewProjection * WorldPos;
}