#version 450

out vec4 FragColor;


in Surface
{
 vec2 WorldPos;
 vec2 TexCoord;
}fs_in;

uniform sampler2D screen;

void main()
{
	vec3 obj_color = texture(screen,fs_in.TexCoord).rgb;
	FragColor = vec4(obj_color,1.0);
}