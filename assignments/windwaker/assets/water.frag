#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
in vec3 toCamera;
in float vs_time;

uniform sampler2D waterTex;

uniform vec3 water_color;

const vec3 reflect_color = vec3(0.0,0.0,0.5);

uniform float tiling;

uniform float b1;
uniform float b2;

void main()
{
	float fresnelFactor =  dot(normalize(toCamera),vec3(0.0,1.0,0.0));

	vec2 dir = normalize(vec2(1.0));
	vec2 uv = (vs_TexCoord * tiling) + dir * vs_time;

	uv.y += 0.01 * (sin(uv.x * 3.5 + vs_time * 0.35) + sin(uv.x * 4.8 + vs_time * 1.05) + sin(uv.x * 7.3 + vs_time * 0.45)) / 3.0;
	uv.x += 0.12 * (sin(uv.y * 4.0 + vs_time * 0.5) + sin(uv.y * 6.8 + vs_time * 0.75) + sin(uv.y * 11.3 + vs_time * 0.2)) / 3.0;
	uv.y += 0.12 * (sin(uv.x * 4.2 + vs_time * 0.64) + sin(uv.x * 6.3 + vs_time * 1.65) + sin(uv.x * 8.2 + vs_time * 0.45)) / 3.0;

	vec4 smp1  = texture(waterTex, uv + vec2(1.0));
	vec4 smp2  = texture(waterTex, uv + vec2(0.2));

	vec3 color = water_color + vec3(smp1.r * b1 - smp2.r * b2);
	FragColor = vec4(color,1.0);
}