#version 450

out vec4 FragColor;
in vec2 vs_TexCoord;
uniform sampler2D _MainTex;
uniform vec2 resolution;
uniform float radius = 0.5;
uniform float softness = 0.02;

vec3 addVignette(vec3 color)
{
    vec2 position = (gl_FragCoord.xy / resolution) - vec2(0.5);           
    float dist = length(position * vec2(resolution.x/resolution.y, 1.0));
    float vignette = smoothstep(radius, radius - softness, dist);

    color -= (1.0 - vignette);

    return color;
}
void main()
{
vec3 albedo = texture(_MainTex,vs_TexCoord).rgb;
albedo = addVignette(albedo);
FragColor = vec4(albedo,1.0);
}