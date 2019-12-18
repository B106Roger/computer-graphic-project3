#version 430 core
uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float time;
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 vs_worldpos;
out vec3 vs_normal;

void main(void)
{
	//position.y= sqrt(position.x * position.x + position.y * position.y)
	vec4 position = proj_matrix * model_matrix * vec4(position, 1.0);
	
    gl_Position = position;
    vs_worldpos = position.xyz;
	vs_normal = mat3(model_matrix) * normal;
}
