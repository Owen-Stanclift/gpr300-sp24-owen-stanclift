#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;

void main()
{
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;
float average = (albedo.r + albedo.g + albedo.b)/3.0;
FragColor = vec4(vec3(average,average,average),1.0);
}