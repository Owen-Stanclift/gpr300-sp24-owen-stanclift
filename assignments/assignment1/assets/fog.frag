#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;
uniform float fog;

float near = 0.1; 
float far  = 100.0; 
  
float linearizeDepth(float depth) 
{
    
    return (near * far)/(far + depth * (near  - far));
}

void main()
{         
    vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;
    float depth = linearizeDepth(gl_FragCoord.z);
    vec4 fogColor = vec4(vec3(fog),1.0);
    vec4 mixColor = mix(fogColor,vec4(albedo,1.0),depth);
    FragColor = mixColor;
}