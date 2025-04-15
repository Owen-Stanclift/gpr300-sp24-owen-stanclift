#version 450

out vec4 FragColor;


in Surface
{
	 vec4 clipSpace;
} fs_in;

//colors
uniform sampler2D reflectTexture;
uniform sampler2D refractTexture;

void main()
{
	vec2 ndc = (fs_in.clipSpace.xy / fs_in.clipSpace.w) / 2.0 + 0.5;
	vec2 refractCoord = vec2(ndc.x, ndc.y);
	vec2 reflectCoord = vec2(ndc.x, -ndc.y);

	vec4 reflectTex = texture(reflectTexture, reflectCoord);
	vec4 refractTex = texture(refractTexture, refractCoord);

	vec4 water_color = mix(reflectTex, refractTex, 0.5);
	FragColor = water_color;
}