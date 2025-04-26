#version 450

out vec4 FragColor;

in Surface
{
	 vec3 normal;
	 vec3 worldPos;
} fs_in;


uniform vec3 color;
uniform vec3 cameraPos;

void main()
{
	vec3 I = normalize(fs_in.worldPos - cameraPos);
	vec3 R = reflect(I,normalize(fs_in.normal);
	FragColor = vec4(mix(color,R).rgb,1.0f);
}