#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;
uniform sampler2D bloomBlur;
uniform float exposure;

void main()
{
const float gamma = 2.2f;
vec3 hdr = texture(_MainTex,vs_TexCoord).rgb;
vec3 bloom = texture(bloomBlur,vs_TexCoord).rgb;
hdr += bloom;

vec3 toneMapped = vec3(1.0f) - exp(-hdr*exposure);
toneMapped = pow(toneMapped,vec3(1.0/gamma));
FragColor = vec4(toneMapped,1.0);
}