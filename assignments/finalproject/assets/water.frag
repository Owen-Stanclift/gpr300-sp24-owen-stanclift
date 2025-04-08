#version 450

out vec4 FragColor;


in Surface
{
	 vec2 TexCoord;
	 vec3 toCamera;
	 vec4 clipSpace;
	 float moveFactor;
}fs_in;

//colors
const vec4 reflectColor = vec4(1.0,0.0,0.0,1.0);
const vec4 refractColor = vec4(0.0,1.0,0.0,0.6);

uniform sampler2D reflectTexture;
uniform sampler2D refractTexture;
uniform sampler2D dudvMap;
uniform float power;

const float waveStrength = 0.02;

void main()
{
	vec2 ndc = (fs_in.clipSpace.xy/fs_in.clipSpace.w)/2.0f + 0.5f;
	vec2 refractCord = ndc;
	vec2 reflectCord = vec2(ndc.x,-ndc.y);

	///////
	vec4 reflectTex = texture(reflectTexture,reflectCord);
	vec4 refractTex = texture(refractTexture,refractCord);

	vec4 distortion1 = (texture(dudvMap,vec2(fs_in.TexCoord.x + fs_in.moveFactor, fs_in.TexCoord.y)) * 2.0 - 1.0) * waveStrength;
	vec4 distortion2 = (texture(dudvMap,vec2(-fs_in.TexCoord.x + fs_in.moveFactor, fs_in.TexCoord.y + fs_in.moveFactor)) * 2.0 - 1.0) * waveStrength;
	vec4 totalDistortion = distortion1 + distortion2;

	reflectTex += totalDistortion;
	reflectTex.x = clamp(reflectTex.x,0.001,0.999);
	reflectTex.y = clamp(reflectTex.y,-0.999,-0.001);

	refractTex += totalDistortion;
	refractTex = clamp(refractTex,0.001,0.999);

	////////
	float refraction = dot(normalize(fs_in.toCamera),vec3(0.0,1.0,0.0));
	float refactionPow = pow(refraction,power);
	vec4 water_color = mix(reflectTex,refractTex,refactionPow);
	FragColor = water_color;
}