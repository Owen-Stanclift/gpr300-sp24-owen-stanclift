#version 450

out vec4 FragColor;


in Surface
{
	 vec3 TexCord;
} fs_in;

//colors
uniform samplerCube skybox;

void main()
{
	
	//FragColor = texture(skybox,fs_in.TexCord);
	FragColor = vec4(1.0,0.0,1.0,1.0);
}