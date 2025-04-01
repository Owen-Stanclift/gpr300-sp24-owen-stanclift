#version 450

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTexCoord;

out Surface
{
    vec2 TexCoord;
}vs_out;

void main()
{

    vs_out.TexCoord = vTexCoord;
    //gl_Position = _CameraViewProjection * world_position;
    gl_Position = vec4(vPos.xy, 0.0, 1.0);
}