#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;

void main()
{
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;
float r = texture(_MainTex,vs_TexCoord - vec2(0.05,0.1)).r;
float g = texture(_MainTex,vs_TexCoord).g;
float b = texture(_MainTex,vs_TexCoord + vec2(0.05,0.1)).b;
FragColor = vec4(r,g,b,1.0);
}