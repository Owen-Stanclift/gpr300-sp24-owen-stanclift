#version 450

out vec4 FragColor;


in Surface
{
	 vec2 TexCord;
	 vec4 clipSpace;
	 vec3 to_camera;
	 vec3 lightVec;
} fs_in;

//colors
uniform sampler2D reflectTexture;
uniform sampler2D refractTexture;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;
uniform vec3 lightColor;

uniform float waveLength;
uniform float moveFactor;
uniform float refractStrength;
void main()
{
	vec2 ndc = (fs_in.clipSpace.xy / fs_in.clipSpace.w) / 2.0 + 0.5;
	vec2 refractCoord = vec2(ndc.x, ndc.y);
	vec2 reflectCoord = vec2(ndc.x, -ndc.y);

	float near = 0.01;
	float far = 1000;
	vec2 distortion1 = texture(dudvMap,vec2(fs_in.TexCord.x + moveFactor,fs_in.TexCord.y)).rg * 2.0 - 1.0;
	vec2 distortion2 = texture(dudvMap,vec2(-fs_in.TexCord.x + moveFactor,fs_in.TexCord.y + moveFactor)).rg * 2.0 - 1.0;
	vec2 totalDistortion = distortion1 + distortion2;

	refractCoord += totalDistortion;
	refractCoord = clamp(reflectCoord,0.001,0.999);

	reflectCoord += totalDistortion;
	reflectCoord.x = clamp(reflectCoord.x,0.001,800.0);
	reflectCoord.y = clamp(reflectCoord.y,-0.999,600.0);

	vec4 normColor = texture(normalMap,totalDistortion);
	vec3 n = vec3(normColor.r*2.0-1.0,normColor.b,normColor.g * 2.0 -1.0);
	n = normalize(n);

	vec3 halfDir = normalize(fs_in.lightVec + fs_in.to_camera);
	float NdotL = max(dot(n,fs_in.lightVec),0.0);
	float NdotH = max(dot(n,halfDir),0.0);

	vec3 reflectLight = reflect(normalize(-fs_in.lightVec),n);
	float spec = pow(NdotH,0.25 * 128);
	vec3 highLights = lightColor * spec * 0.5;

	vec4 reflectTex = texture(reflectTexture, reflectCoord);
	vec4 refractTex = texture(refractTexture, refractCoord);
	float refractFactor = dot(normalize(fs_in.to_camera),vec3(0.0,1.0,0.0));
	refractFactor = pow(refractFactor,refractStrength);

	vec4 water_color = mix(reflectTex, refractTex, refractFactor);

	FragColor = water_color;
	FragColor += mix(FragColor,vec4(0,0.3,0.5,1.0),0.2);
	FragColor += vec4(highLights,0.0);
}