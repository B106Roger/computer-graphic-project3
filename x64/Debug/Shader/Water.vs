#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float time;

layout(location = 0) in vec2 position;
out vec2 textureCoorVs;
void main(void)
{
	vec3 displacement = vec3(-100,0,0);
	textureCoorVs = position.xy + vec2(100,100);
	textureCoorVs = textureCoorVs / 200;
	float wave = 3.f;
	float distance= sqrt(position.x * position.x + position.y * position.y);
	float height = cos(1000.f - distance / 4.f + time) * wave;
	vec3 new_position = vec3(
	position.x+displacement.x,
	height+displacement.y, 
	position.y+displacement.z);
	
    gl_Position = proj_matrix * model_matrix * vec4(new_position, 1.0);
}
