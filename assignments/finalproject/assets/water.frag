#version 450

out vec4 FragColor;


in Surface
{
	 vec2 TexCord;
	 vec4 clipSpace;
	 vec3 to_camera;
	 vec3 lightVec;
	 vec3 normal;
} fs_in;

//colors
uniform sampler2D reflectTexture;
uniform sampler2D refractTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform vec3 lightColor;

uniform samplerCube skybox;

uniform float waveLength;
uniform float moveFactor;
uniform float refractStrength;
void main()
{
	vec3 ndc = (fs_in.clipSpace.xyz / fs_in.clipSpace.w) / 2.0 + 0.5;
	vec2 refractCoord = vec2(ndc.x, ndc.y);
	vec2 reflectCoord = vec2(ndc.x, -ndc.y);

	float near = 0.01;
	float far = 1000;
	//moveFactor = clamp(moveFactor,0,1);
	vec2 distortion = (texture(dudvMap,vec2((fs_in.TexCord.x) + (moveFactor),fs_in.TexCord.y)).rg * 0.1) ;
	distortion *= (texture(dudvMap,vec2((-fs_in.TexCord.x) + (moveFactor) ,(fs_in.TexCord.y) + (moveFactor))).rg * 0.1);
//
	refractCoord += distortion;
	//refractCoord = clamp(reflectCoord,0.001,0.999);

	reflectCoord += distortion;
	//reflectCoord.x = clamp(reflectCoord.x,0.001,0.999);
	//reflectCoord.y = clamp(reflectCoord.y,-0.999,-0.001);

	vec4 normColor = texture(normalMap,distortion);
	vec3 n = vec3(normColor.r*2.0-1.0,normColor.b,normColor.g * 2.0 -1.0);
	n = normalize(n);

	vec3 halfDir = normalize(-100*fs_in.lightVec + fs_in.to_camera);
	float NdotL = max(dot(n,fs_in.lightVec),0.0);
	float NdotH = max(dot(n,halfDir),0.0);

	vec3 reflectLight = reflect(normalize(-fs_in.lightVec),n);
	float spec = pow(NdotH,0.25 * 128);
	vec3 highLights = lightColor * spec * 0.5;
//
	vec4 reflectTex = texture(reflectTexture, reflectCoord);
	vec4 refractTex = texture(refractTexture, refractCoord);
	float refractFactor = dot(normalize(fs_in.to_camera),vec3(0.0,1.0,0.0));
	refractFactor = pow(refractFactor,refractStrength);

	vec4 water_color = mix(reflectTex, refractTex, refractFactor);

	FragColor = water_color;
	FragColor = mix(FragColor,vec4(0,0.3,0.5,1.0),0.2) + vec4(highLights,0.0);
	//FragColor += vec4(texture(skybox,R).rgb,1.0);
}