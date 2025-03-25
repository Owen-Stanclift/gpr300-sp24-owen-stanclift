#version 450

out vec4 FragColor;


in Surface
{
 vec2 TexCoord;
}fs_in;

struct Light {
  vec3 position;
  vec3 color;
};

uniform Light lights[64];


uniform sampler2D albedo;
uniform sampler2D position;
uniform sampler2D normal;

uniform vec3 cameraPos;

void main()
{
	vec3 FragPos = texture(position,fs_in.TexCoord).rgb;
	vec3 norm = texture(normal,fs_in.TexCoord).rgb;
	vec3 objCol = texture(albedo,fs_in.TexCoord).rgb;
	vec3 light = vec3(0);
	vec3 viewDir = normalize(cameraPos - FragPos);
	for(int i = 0; i < 64;i++)
	{
		vec3 lightDir = normalize(lights[i].position - FragPos);
		vec3 diffuse = max(dot(norm,lightDir),0.0) * lights[i].color;
		light += diffuse;
	}

	FragColor = vec4(objCol * light,1.0);
}