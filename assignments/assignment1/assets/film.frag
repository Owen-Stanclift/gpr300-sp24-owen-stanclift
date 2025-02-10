#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;


float noiseAmount = 0.1f;
uniform float iTime;

void main()
{
vec4 tex = texture(_MainTex,vs_TexCoord);
float noise = (fract(sin(dot(vs_TexCoord, vec2(12.9898,78.233)*2.0)) * iTime));
FragColor = tex - (noise * noiseAmount);
}