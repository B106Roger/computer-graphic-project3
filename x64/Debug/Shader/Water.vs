#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float time;

layout(location = 0) in vec3 position;

void main(void)
{
	float wave = 3.f;
	float distance= sqrt(position.x * position.x + position.z * position.z);
	float height = cos(1000.f - distance / 5.f + time) * wave;
	vec3 new_position = vec3(position.x, height, position.z);
	
    gl_Position = proj_matrix * model_matrix * vec4(new_position, 1.0);
}
