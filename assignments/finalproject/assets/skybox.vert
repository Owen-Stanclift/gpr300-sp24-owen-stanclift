#version 450

layout(location = 0) in vec3 vPos;

out Surface
{
	vec3 TexCord;
}vs_out;

uniform mat4 view_proj;

void main()
{
	vec4 WorldPos = view_proj * vec4(vPos,1.0f);
	vs_out.TexCord = vec3(vPos.x,vPos.y,-vPos.z);
	gl_Position =  vec4(WorldPos.x,WorldPos.y,WorldPos.w,WorldPos.w);
}