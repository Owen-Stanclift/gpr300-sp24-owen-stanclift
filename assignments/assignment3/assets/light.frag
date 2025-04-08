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
uniform float radius;

uniform sampler2D albedo;
uniform sampler2D position;
uniform sampler2D normal;

uniform vec3 cameraPos;

float expoAttenutate(float dist, float rad)
{
	float i = clamp(1.0 - pow((dist / rad), 4.0), 0.0, 1.0);
	return i*i;
}

vec3 blinnPhong(Light light, vec3 fragpos, vec3 normal)
{
	vec3 view_dir = normalize(cameraPos - fragpos);
    vec3 light_dir = normalize(light.position - fragpos);
	vec3 half_way = normalize(light_dir + view_dir);

	float NdotL = max(dot(normal, light_dir), 0.0);
	float NdotH = max(dot(normal, half_way), 0.0);

	vec3 diffuse = NdotL * vec3(0.5);
	vec3 specular = pow(NdotH, 0.5 * 128.0) * vec3(0.5);

	float attenuation = expoAttenutate(length(light_dir), radius);
	return (diffuse + specular) * light.color;
}

void main()
{
	vec2 uv = fs_in.TexCoord;
	vec3 FragPos = texture(position, uv).rgb;
	vec3 norm = texture(normal, uv).rgb;
	vec3 objCol = texture(albedo, uv).rgb;

//	vec3 lighting = vec3(0);
//	for(int i = 0; i < 64;i++)
//	{
//		lighting += blinnPhong(vec, FragPos, norm);
//  }

	
	Light test;
	test.position = vec3(5.0, 5.0, 0.0);
	test.color = vec3(1.0, 0.0, 0.0);

	vec3 light_dir = normalize(test.position - FragPos);
	vec3 lighting = blinnPhong(test, FragPos, norm);
	FragColor = vec4(lighting, 1.0);
}