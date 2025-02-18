#version 450

out vec4 FragColor;


in Surface
{
 vec3 WorldPos;
 vec4 LightPos;
 vec3 WorldNormal;
 vec2 TexCoord;
}fs_in;


uniform sampler2D _MainTex;
uniform sampler2D shadow_map;
uniform vec3 _EyePos;
uniform vec3 camera_pos;
uniform vec3 light_pos = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material
{
	vec3 Ka; //Ambience (0-1)
	vec3 Kd; //Diffuse (0-1)
	vec3 Ks; //Specular (0-1)
	float alpha;
	float Shininess; //Size of specular
};
uniform Material _Material;


float shadow_calc(vec4 frag_pos_lightspace)
{
vec3 proj_cord = frag_pos_lightspace.xyz/ frag_pos_lightspace.w;
proj_cord = (proj_cord * 0.5) + 0.5;

float current_depth = texture(shadow_map, proj_cord.xy).r;
float closest_depth = proj_cord.z;
float shadow = (current_depth > closest_depth) ? 1.0 : 0.0;
	return shadow;;
}
vec3 blinphong(vec3 normal, vec3 frag_pos)
{
	vec3 view_dir = normalize(camera_pos - frag_pos);
	vec3 light_dir = normalize(light_pos - frag_pos);
	vec3 halfway_dir = normalize(light_dir + view_dir);

	float ndot1 = max(dot(normal,light_dir),0.0);
	float ndoth = max(dot(normal,halfway_dir),0.0);

	vec3 diffuse = ndot1 * _Material.Kd;
	vec3 spec = pow(ndoth,_Material.Shininess * 128) * _Material.Ks;

	return (diffuse + spec);
}
void main()
{
vec3 normal = normalize(fs_in.WorldNormal);
float shadow = shadow_calc(fs_in.LightPos);
vec3 light  = blinphong(normal,fs_in.WorldPos);
light *= (1.0 - shadow);
light *= _LightColor;
light *= vec3(1.0) * _Material.Ka;

vec3 obj_color = normal *0.5 +0.5;
FragColor = vec4(obj_color * light,1.0);
}