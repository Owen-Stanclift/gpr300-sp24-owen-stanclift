#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _Model;
uniform mat4 _CameraViewProjection;
uniform mat4 _LightViewProjection;

out Surface
{
vec4 WorldPos;
vec4 LightPos;
vec3 WorldNormal;
vec2 TexCoord;
}vs_out;

void main()
{
vs_out.WorldPos = vec3(_Model * vec4(vPos,1.0));
vs_out.LightPos = _LightViewProjection * vs_out.WorldPos;
vs_out.WorldNormal = transpose(inverse(mat3(_Model))) *vNormal;
vs_out.TexCoord = vTexCoord;
gl_Position = _CameraViewProjection * vs_out.WorldPos;
}