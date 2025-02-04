#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;


float near = 0.1; 
float far  = 100.0; 
  
float linearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (z*(far-near)-(far+near));	
}

void main()
{         
    float depth = linearizeDepth(gl_FragCoord.z);
    FragColor = vec4(vec3(depth), 1.0);
}