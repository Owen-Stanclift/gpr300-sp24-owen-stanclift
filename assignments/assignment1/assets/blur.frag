#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;

const float offset = 1.0f/300.0f;
const vec2 offsets[9] = vec2[](
vec2(-offset,offset),
vec2(0.0, offset),
vec2(offset,offset),

vec2(-offset,0),
vec2(0.0,0),
vec2(offset,0),

vec2(-offset,-offset),
vec2(0.0,-offset),
vec2(offset,-offset)
);

const float strength = 16.0f;
const float kernel[9]= float[](
1.0,1.0,1.0,
1.0,1.0,1.0,
1.0,1.0,1.0
);

void main()
{
vec3 avg = vec3(0.0);

for(int i =0; i < 9; i++)
{
vec3 local = texture(_MainTex,vs_TexCoord + offsets[i]).rgb;
avg += local * (kernel[i]/strength);
}
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;
float average = (avg.r + avg.g + avg.b, 1.0)/9.0;
FragColor = vec4(avg,1.0);
}