#version 450

out vec4 FragColor;
const float pi = 3.1415926459;

in Surface
{
 vec3 WorldPos;
 vec3 normal;
 vec2 TexCoord;
}fs_in;

struct Lights {
  vec3 position;
  vec3 color;
};

uniform sampler2D screen;
uniform vec3 camera_position;
uniform Lights[] light;
uniform int num_of_lights;

vec3 BDRF(vec3 pos, vec3 dir)
{
	return vec3(1.0,0.0,0.0);
}
vec3 outgoing_light(vec3 outDir, vec3 fragPos)
{
	vec3 emitted;
	vec3 radiance;

	for(int i = 0; i < num_of_lights; i++)
	{
		vec3 incoming = light[i].color;
		vec3 lightDir = normalize(light[i].position - fragPos);
		vec3 bdrf = BDRF(fs_in.WorldPos,lightDir);
		float dotL = dot(fs_in.normal,lightDir);

		radiance += bdrf * incoming * dotL;
	}
	return radiance  + emitted;
}

void main()
{
		vec3 viewDir = normalize(camera_position - fs_in.WorldPos);
		vec3 bdrfLight = outgoing_light(viewDir,fs_in.WorldPos);
		FragColor = vec4(bdrfLight,1.0);
}