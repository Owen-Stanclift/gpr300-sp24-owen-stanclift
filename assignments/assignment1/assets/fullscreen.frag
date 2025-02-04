#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;

void main()
{
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;

FragColor = vec4(albedo,1.0);
}