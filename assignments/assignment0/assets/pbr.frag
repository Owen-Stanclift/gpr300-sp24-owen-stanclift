#version 450

out vec4 FragColor;
float PI  = 3.1415926535;

in Surface
{
 vec3 WorldPos;
 vec3 WorldNormal;
 vec2 TexCoord;
}fs_in;



uniform sampler2D _MainTex;
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0f);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);
uniform float metallic;
uniform float roughness;

struct Material
{
	vec3 Ka; //Ambience (0-1)
	vec3 Kd; //Diffuse (0-1)
	vec3 Ks; //Specular (0-1)
	float alpha;
	float Shininess; //Size of specular
};
uniform Material _Material;

float VdotN = 0.0;
float LdotN = 0.0;
float NdotH = 0.0;

vec3 F() {return vec3(1.0f);};
vec3 G() {return vec3(1.0f);};
vec3 D(float roughness) 
{
	float a2 = pow(roughness,4.0);
	float denom = PI * pow(pow(NdotH, 2.0) * (a2 - 1.0) + 1.0, 2.0);
	return vec3(a2/denom);
}



vec3 cookTor(vec3 fresnel, float roughness)
{
	vec3 num = D(roughness) * fresnel *G();
	float denom = 4.0 * VdotN * LdotN;
	return num/denom;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
	float a2 = a*a;
	float dotH = max(dot(N,H),0.0);
	float dotH2 = dotH*dotH;

	float nom = a2;
	float denom = (dotH2 * (a2 - 1.0) +1.0);
	denom = PI * denom * denom;

	return nom/denom;
}

float SchlickGGX(float dotV, float k)
{
	float nom = dotV;
	float denom = dotV * (1.0f - k) +k;

	return nom/denom;
}
float Smith(vec3 N, vec3 V, vec3 L, float k)
{
	float dotV = max(dot(N,V),0.0);
	float dotL = max(dot(N,L),0.0);
	float ggx1 = SchlickGGX(dotV,k);
	float ggx2 = SchlickGGX(dotL,k);
	return ggx1*ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0-cosTheta,5.0);
}

vec3 BDRF(vec3 pos, vec3 dir)
{
	vec3 normal = normalize(fs_in.WorldNormal);

	vec3 lambert = vec3(1.0, 0.0, 0.0)/PI;

	vec3 KS = fresnelSchlick(NdotH,lambert);
	vec3 KD = (1 - KS) * (1- metallic);

	vec3 diffuse = (KD * lambert);
	vec3 specular = KS * cookTor(KS,roughness); //KS; 

	return  specular;
}

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal);

	vec3 toLight = -_LightDirection;
	NdotH = dot(normal,toLight);
//	float diffuseFactor = 0.5 * max(dot(normal,toLight),0.0);
//	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
//	vec3 h = normalize(toLight + toEye);
//	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
//	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
//	lightColor += _AmbientColor + _Material.Ka;
	//vec3 objectColor = texture (_MainTex,fs_in.TexCoord).rgb;

	LdotN = dot(normal, toLight);
	vec3 final = BDRF(fs_in.WorldPos,toLight) * _LightColor * LdotN;
	//final = mix(final,objectColor,1.0f);
	FragColor = vec4(final, 1.0);
}