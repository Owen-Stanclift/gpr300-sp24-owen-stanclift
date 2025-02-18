#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 _Model;
uniform mat4 _ViewProjection;
uniform vec3 cameraPosition;
uniform float time;

out vec2 vs_TexCoord;
out vec3 toCamera;
out float vs_time;


float calcSurface(float x, float z)
{
  float scale = 30.0;
  float y = 0.0;
  y += (sin(x * 1.0 / scale + time * 1.0) + sin(x * 2.3 / scale + time * 1.5) + sin(x * 3.3 / scale + time * 0.4)) / 3.0;
  y += (sin(z * 0.2 / scale + time * 1.8) + sin(z * 1.8 / scale + time * 1.8) + sin(z * 2.8 / scale + time * 0.8)) / 3.0;
  return y;
}

void main()
{
	vec3 pos = vPos;
	//pos.y = calcSurface(pos.x,pos.z);
	vec4 world_position = _Model * vec4(pos,1.0); 

	toCamera = cameraPosition - world_position.xyz;
	vs_TexCoord = vTexCoord;
	vs_time = time;
	gl_Position = _ViewProjection * world_position;
}

