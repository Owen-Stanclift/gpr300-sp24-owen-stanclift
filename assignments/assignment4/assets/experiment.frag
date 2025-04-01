#version 450

layout(location = 0) out vec4 FragAlbedo;
layout(location = 1) out vec4 FragPosition;
layout(location = 2) out vec4 FragNormal;

in Surface
{
 vec3 WorldPos;
 vec3 WorldNormal;
 vec2 TexCoord;
}fs_in;

void main()
{
	vec3 objColor = fs_in.WorldNormal.xyz * 0.5 +0.5;
	FragAlbedo = vec4(objColor,1.0);
	FragPosition = vec4(fs_in.WorldPos, 1.0);
	FragNormal = vec4(fs_in.WorldNormal, 1.0);
}