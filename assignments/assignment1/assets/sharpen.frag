#version 450


out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;

const float kernel[9]= float[](
0.0,-1.0,0.0,
-1.0,5.0,-1.0,
0.0,-1.0,0.0
);

const vec2 directions[9] = vec2[](
vec2(-1,1),vec2(0,1),vec2(1,1),
vec2(-1,0),vec2(0,0),vec2(1,0),
vec2(-1,-1),vec2(0,-1),vec2(1,-1)
);

void main()
{
vec2 size = 1.0 / textureSize(_MainTex,0).xy;
vec3 avg = vec3(0.0);

for(int i = 0; i < 9; i++)
{
	vec2 offset = directions[i]*size;
	vec3 color = texture(_MainTex,vs_TexCoord+offset).rgb;
	avg += color * (kernel[i]);
}
FragColor = vec4(avg,1.0);
}