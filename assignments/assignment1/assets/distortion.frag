#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;


float strength = 0.2f;
uniform float iTime;
uniform vec2 aspect;

void main()
{
vec2 center = vec2(0.5);
vec2 dir = center - vs_TexCoord;

float d = length(dir / aspect) - strength * sin(iTime);
d *= 0.5 - smoothstep(0.0f,0.1f,abs(d));
dir = normalize(dir);

FragColor = texture(_MainTex,vs_TexCoord + dir * d);
}