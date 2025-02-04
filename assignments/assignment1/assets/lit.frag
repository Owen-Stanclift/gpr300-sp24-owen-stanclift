#version 450

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_bright;

in Surface
{
 vec3 WorldPos;
 vec3 WorldNormal;
 vec2 TexCoord;
}fs_in;


uniform sampler2D _MainTex;
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(100.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material
{
	vec3 Ka; //Ambience (0-1)
	vec3 Kd; //Diffuse (0-1)
	vec3 Ks; //Specular (0-1)
	float alpha;
	float Shininess; //Size of specular
};
uniform Material _Material;

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal);

	vec3 toLight = -_LightDirection;

	float diffuseFactor = 0.5 * max(dot(normal,toLight),0.0);

	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);

	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);

	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor += _AmbientColor + _Material.Ka;
	vec3 objectColor = texture (_MainTex,fs_in.TexCoord).rgb;

	frag_color = vec4(objectColor * lightColor,1.0);

	//brightness

	float brightness = dot(frag_color.rgb, vec3(0.2126,0.7152,0.0722));
	if(brightness > 1.0)
	{
		frag_bright = frag_color;
	}
	else
	{
		frag_bright = vec4(0.0);
	}
}