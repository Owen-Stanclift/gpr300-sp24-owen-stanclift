#version 450

out vec4 FragColor;


in Surface
{
	 vec2 TexCoord;
	 vec3 toCamera;
}fs_in;

//colors
const vec4 reflectColor = vec4(1.0,0.0,0.0,1.0);
const vec4 refractColor = vec4(0.0,1.0,0.0,0.6);

void main()
{
	float refraction = dot(normalize(fs_in.toCamera),vec3(0.0,1.0,0.0));

	vec4 water_color = mix(reflectColor,refractColor,refraction);
	FragColor = water_color;
}