#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _Model;
uniform mat4 _CameraViewProjection;
uniform mat4 _LightViewProjection;

out Surface
{
    vec3 WorldPos;
    vec4 LightPos;
    vec3 WorldNormal;
    vec2 TexCoord;
}vs_out;

void main()
{
    vec4 world_position = _Model * vec4(vPos,1.0);
    vs_out.WorldPos = world_position.xyz;

    vs_out.LightPos = _LightViewProjection * world_position;
    vs_out.WorldNormal = transpose(inverse(mat3(_Model))) *vNormal;
    vs_out.TexCoord = vTexCoord;
    gl_Position = _CameraViewProjection * world_position;
}