#version 450

out vec4 FragColor;

in Surface
{
    vec2 TexCoord;
    vec4 LightSpacePosition;
    vec3 WorldPosition;
}fs_in;

uniform sampler2D heightmap;
uniform sampler2D shadowmap;
uniform vec3 cameraPos;

struct LandmassProp
{
	float scale;
};
uniform LandmassProp landmass;

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

float shadow_calc(vec4 frag_pos_lightspace)
{
	vec3 proj_cord = frag_pos_lightspace.xyz / frag_pos_lightspace.w;
	proj_cord = (proj_cord * 0.5) + 0.5;

	float closest_depth = texture(shadowmap, proj_cord.xy).r;
	float current_depth = proj_cord.z;

	float shadow = 0.0f;
	vec2 texSize = 1.0/ textureSize(shadowmap, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
		    float pcfDepth = texture(shadowmap, proj_cord.xy + vec2(x, y) * texSize).r;
		    shadow += (current_depth - 0.05) > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;
	return shadow;
}

// n: height
vec3 getTerrainColor(float n)
{
    //FragColor = shadow * normShadow;
	if (n <= 0.3) {
        // Light teal or watery blue
        return vec3(0.635, 0.561, 0.455);
    } else if (n <= 0.4) {
        // Soft sandy beige (light warm sand)
        return vec3(0.8392, 0.7137, 0.6196);
    } else if (n <= 0.5) {
        // Fresh green (light mossy green)
        return vec3(0.5961, 0.6784, 0.3529);
    } else if (n <= 0.6) {
        // Olive green (earthy green with brown undertones)
        return vec3(0.3961, 0.5216, 0.2549);
    } else if (n <= 0.7) {
        // Darker green (dense forest or deep foliage green)
        return vec3(0.2784, 0.4627, 0.2706);
    } else if (n <= 0.8) {
        // Grayish blue (slightly muted, steel gray with blue)
        return vec3(0.4275, 0.4627, 0.5294);
    } else if (n <= 0.9) {
        // Soft grayish blue (calming gray with a hint of blue)
        return vec3(0.5176, 0.5529, 0.6039);
    } else {
        // Light gray with a touch of green (light and muted, almost neutral)
        return vec3(0.8235, 0.8784, 0.8706);
    }
}

void main()
{
	float n = 0.0;
	for(int i = 0; i <9; i++)
	{
    	vec2 uv = fs_in.TexCoord + offsets[i];
		float local = texture(heightmap, uv).r;
		n += local * (kernel[i] / strength);
	}

    // procedural coloring
    vec3 color = getTerrainColor(n);

    // shadowing
    float shadow = shadow_calc(fs_in.LightSpacePosition);
    color *= (1.0 - shadow);

    FragColor = vec4(color, 1.0);
}