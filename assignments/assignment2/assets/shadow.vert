#version 450

layout(location = 0) in vec3 vPos;

uniform mat4 _Model;
uniform mat4 _LightViewProjection;

void main()
{
vec4 world_pos = _Model * vec4(vPos,1.0);
gl_Position = _LightViewProjection * world_pos;
}