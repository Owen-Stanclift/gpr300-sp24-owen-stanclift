#version 450

out vec4 FragColor;


in Surface
{
 vec3 WorldPos;
 vec3 WorldNormal;
 vec2 TexCoord;
}fs_in;


uniform sampler2D _Albedo;
uniform sampler2D _ZATOON;
uniform vec3 shadow;
uniform vec3 highlight;
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);

vec3 toon_lighting(vec3 normal, vec3 light_dir)
{
	float diff = (dot(normal,light_dir) + 1.0) * 0.5;
	float step = texture(_ZATOON, vec2(diff)).r;
	vec3 light_color = mix(shadow,highlight,step);
	return light_color * step;
}

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -_LightDirection;
	vec3 light_color = toon_lighting(normal, toLight);
	vec3 object_color = texture(_Albedo, fs_in.TexCoord).rgb;
	FragColor = vec4(object_color * light_color,1.0);
}