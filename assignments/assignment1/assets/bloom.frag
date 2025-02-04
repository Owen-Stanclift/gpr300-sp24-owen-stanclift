#version 450


out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;
uniform float weight[5] = float[](0.227,0.195,0.122,0.054,0.016);
uniform bool horizontal;
void main()
{
vec2 size = 1.0 / textureSize(_MainTex,0).xy;
vec3 color = texture(_MainTex,vs_TexCoord).rgb * weight[0];

if(horizontal)
{
	for(int i =0;i <5; i++)
	{
		color += texture(_MainTex,vs_TexCoord + vec2(size.x * i,0.0)).rgb * weight[i]; 
		color += texture(_MainTex,vs_TexCoord - vec2(size.x * i,0.0)).rgb * weight[i]; 
	}
}
else
{
		for(int i =0;i <5; i++)
	{
		color += texture(_MainTex,vs_TexCoord + vec2(size.y * i,0.0)).rgb * weight[i]; 
		color += texture(_MainTex,vs_TexCoord - vec2(size.y * i,0.0)).rgb * weight[i]; 
	}
}
FragColor = vec4(color,1.0);
}