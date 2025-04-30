#version 450

out vec4 FragColor;


in Surface
{
    vec2 TexCoord;
    vec3 lightDirection;
    vec3 viewDirection;
    vec3 WorldPosition;
}fs_in;

uniform sampler2D heightmap;
uniform sampler2D normalmap;
uniform vec3 cameraPos;
uniform vec4 lightColor;
struct LandmassProp
{
	float scale;
};
uniform LandmassProp landmass;

float inShadow;

//Calculations:
const float offset = 1.0 / 2049.0;
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

vec4 getTerrainColor(float n)
{
    //FragColor = shadow * normShadow;
	if (n <= 0.3) {
        // Light teal or watery blue
        return vec4(0.635, 0.561, 0.455, 1.0);
    } else if (n <= 0.4) {
        // Soft sandy beige (light warm sand)
        return vec4(0.8392, 0.7137, 0.6196, 1.0);
    } else if (n <= 0.5) {
        // Fresh green (light mossy green)
        return vec4(0.5961, 0.6784, 0.3529, 1.0);
    } else if (n <= 0.6) {
        // Olive green (earthy green with brown undertones)
        return vec4(0.3961, 0.5216, 0.2549, 1.0);
    } else if (n <= 0.7) {
        // Darker green (dense forest or deep foliage green)
        return vec4(0.2784, 0.4627, 0.2706, 1.0);
    } else if (n <= 0.8) {
        // Grayish blue (slightly muted, steel gray with blue)
        return vec4(0.4275, 0.4627, 0.5294, 1.0);
    } else if (n <= 0.9) {
        // Soft grayish blue (calming gray with a hint of blue)
        return vec4(0.5176, 0.5529, 0.6039, 1.0);
    } else {
        // Light gray with a touch of green (light and muted, almost neutral)
        return vec4(0.8235, 0.8784, 0.8706, 1.0);
    }

}

float calcLight(vec3 normal)
{
    float diffuse = max(dot(normal,fs_in.lightDirection),0);
    float specLight = 0.5;
    vec3 reflectDirection = reflect(-fs_in.lightDirection,normal);
    float specAmount = pow(max(dot(fs_in.viewDirection,reflectDirection),0),3);
    float spec = specAmount * specLight;
    
    float result = diffuse + spec;
    return result;
}

void main()
{
	float n = 0.0;
	for(int i = 0; i <9; i++)
	{
    	vec2 hPos = fs_in.TexCoord + offsets[i];
		float local = texture(heightmap,hPos).r;
		n += local * (kernel[i]/strength);
	}

    vec3 normal = texture(normalmap,fs_in.TexCoord).rgb;
    normal = normalize(normal*2-1);

    vec4 shadowColor = vec4(vec3(0),1);

    FragColor = vec4(getTerrainColor(n).rgb * lightColor.rgb * calcLight(normal),1);
}