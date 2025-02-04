#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;
uniform float exposure = 1.0f;
void main()
{
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;


vec3 toneMapped = vec3(1.0f) - exp(-albedo*exposure);
FragColor = vec4(toneMapped,1.0);
}