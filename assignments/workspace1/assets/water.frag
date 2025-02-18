#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
in vec3 toCamera;
in float vs_time;

uniform sampler2D waterTex;
uniform sampler2D water_spec;
uniform sampler2D water_warp;

uniform vec3 water_color;

const vec3 reflect_color = vec3(0.0,0.0,0.5);

uniform float scale;
uniform float spec_scale;
uniform float warp_strength;
uniform float brightness_lower_cutoff;
uniform float brightness_upper_cutoff;

void main()
{
	vec2 uv = vs_TexCoord;

	vec2 warp_uv = vs_TexCoord * scale;
	vec2 warp_scroll  = vec2(0.5,0.5) * vs_time;
	vec2 warp = texture(water_warp, warp_uv + warp_scroll).xy * warp_strength;
	warp = (warp*2.0) - 1.0;

	vec2 albedo_uv = vs_TexCoord * scale;
	vec2 albedo_scroll = vec2(-0.5,0.5) * vs_time;
	vec4 albedo = texture(waterTex,albedo_uv+ warp + albedo_scroll);

	vec2 spec_uv = vs_TexCoord * spec_scale;
	vec3 smp1 = texture(water_spec,spec_uv + vec2(1.0,0.0)*vs_time).rgb;
	vec3 smp2 = texture(water_spec,spec_uv + vec2(1.0,1.0)*vs_time).rgb;
	vec3 spec = smp1 + smp2;

	//float brightness = dot(spec,vec3(0.299,0.597,0.114));
	//if(brightness <= brightness_lower_cutoff || brightness >= brightness_upper_cutoff)
	//{
	//discard;
	//}

	//float fresnel = dot(normalize(toCamera),vec3(0.0,1.0,0,0));
	//vec3 finalColor = mix(spec, water_color + vec3(albedo.a),fresnel);
	FragColor = vec4(water_color + vec3(albedo.a), 1.0);
}