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
uniform vec3 mainLightPos;


float attenutate(float dist, float rad)
{
	return clamp((rad-dist)/rad,0.0,1.0);
}
float expoAttenutate(float dist, float rad)
{
	float i = clamp(1.0 - pow((rad-dist)/rad,4.0),0.0,1.0);
	return i*i;
}

vec3 calcPointLight(Light lights, vec3 norm, vec3 pos)
{
	vec3 diff = lights.position - pos;
	vec3 toLight = normalize(diff);
	float d = length(diff);
	vec3 lightDir = normalize(lights.position - pos);
	vec3 lightColor = max(dot(norm,lightDir),0.0) * lights.color;
	lightColor *= expoAttenutate(d,radius);
	return lightColor;
}
vec3 calcDirLight(vec3 norm, vec3 pos)
{
	vec3 lightDir = normalize(mainLightPos - pos);
	vec3 viewDir = normalize(cameraPos - pos); //I finally get why we had this. Almost every tutorial I read leaves out directional light calculation.
	vec3 halfDir = normalize(lightDir + viewDir);
	return halfDir;
}
void main()
{
	vec3 FragPos = texture(position,fs_in.TexCoord).rgb;
	vec3 norm = texture(normal,fs_in.TexCoord).rgb;
	vec3 objCol = texture(albedo,fs_in.TexCoord).rgb;
	vec3 light = vec3(0);
	
	light = calcDirLight(norm,FragPos);
	for(int i = 0; i < 64;i++)
	{
		light += calcPointLight(lights[i],norm,FragPos);
	}

	FragColor = vec4(objCol * light,1.0);
}