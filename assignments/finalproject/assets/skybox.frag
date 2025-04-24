#version 450

out vec4 FragColor;


in Surface
{
	 vec3 TexCord;
} fs_in;

//colors
uniform samplerCube skybox;
vec4 skycolor = vec4(0.0,0.0,0.0,0.0);
void main()
{
	
	vec4 sky = texture(skybox,fs_in.TexCord);
	FragColor = mix(sky,skycolor,0.5);
}