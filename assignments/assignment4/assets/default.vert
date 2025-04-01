#version 450

layout(location = 0) in vec3 vPos;
layout(location = 0) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;


out Surface
{
vec3 WorldPos;
vec3 normal;
vec2 TexCoord;
}vs_out;

void main()
{
vs_out.WorldPos = vPos;
vs_out.normal = vNormal;
vs_out.TexCoord = vTexCoord;
gl_Position = vec4(vPos,0,1.0);
}