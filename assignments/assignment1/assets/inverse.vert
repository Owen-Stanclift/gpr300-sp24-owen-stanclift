#version 450

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTexCoord;

out vec2 vs_TexCoord;

void main()
{
vs_TexCoord = vTexCoord;
gl_Position = vec4(vPos.xy,0.0,1.0);
}