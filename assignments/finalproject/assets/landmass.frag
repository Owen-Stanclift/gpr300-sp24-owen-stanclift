#version 450

out vec4 FragColor;


in Surface
{
    vec2 TexCoord;
    float inShadow;
    float normalShadow;
    vec4 lightPosition;
    vec3 WorldPosition;
}fs_in;

uniform sampler2D depth;
uniform sampler2D heightmap;
uniform vec3 cameraPos;

//Calculations:
const float offset = 1.0 / 2048.0;
const vec2 offsets[9] = vec2[]( 
    vec2(-offset,  offset), // top-left
    vec2( 0.0,     offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0),    // center-left
    vec2( 0.0,     0.0),    // center-center
    vec2( offset,  0.0),    // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0,    -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right    
);

const float strength = 16.0;
const float kernel[9] = float[](
    1.0, 2.0, 1.0,
    2.0, 4.0, 2.0,
    1.0, 2.0, 1.0 
);


void main()
{
    vec2 ndc = (fs_in.lightPosition.xy / fs_in.lightPosition.w) / 2.0 + 0.5;
	vec2 shadowCord = vec2(ndc.x, ndc.y);
	float n = 0.0;
	for(int i = 0; i <9; i++)
	{
		float local = texture(heightmap,fs_in.TexCoord + offsets[i]).r;
		n += local * (kernel[i]/strength);
	}
    vec4 shadowCol = vec4(0.0,0.0,0.0,1);

    float normShadow = clamp(fs_in.inShadow+fs_in.normalShadow,0,1);

    vec4 shadow = texture(depth,ndc);
    //FragColor = shadow * normShadow;
	if (n <= 0.3) {
        // Light teal or watery blue
        FragColor = vec4(0.635, 0.561, 0.455, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.4) {
        // Soft sandy beige (light warm sand)
        FragColor = vec4(0.8392, 0.7137, 0.6196, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.5) {
        // Fresh green (light mossy green)
        FragColor = vec4(0.5961, 0.6784, 0.3529, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.6) {
        // Olive green (earthy green with brown undertones)
        FragColor = vec4(0.3961, 0.5216, 0.2549, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.7) {
        // Darker green (dense forest or deep foliage green)
        FragColor = vec4(0.2784, 0.4627, 0.2706, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.8) {
        // Grayish blue (slightly muted, steel gray with blue)
        FragColor = vec4(0.4275, 0.4627, 0.5294, 1.0);
        FragColor.rgb *= normShadow;
    } else if (n <= 0.9) {
        // Soft grayish blue (calming gray with a hint of blue)
        FragColor = vec4(0.5176, 0.5529, 0.6039, 1.0);
        FragColor.rgb *= normShadow;
    } else {
        // Light gray with a touch of green (light and muted, almost neutral)
        FragColor = vec4(0.8235, 0.8784, 0.8706, 1.0);
        FragColor.rgb *= normShadow;
    }
}