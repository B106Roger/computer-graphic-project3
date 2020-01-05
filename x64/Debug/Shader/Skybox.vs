#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;

layout(location = 0) in vec3 position;
out vec3 TexCoords;


void main(void)
{	
	TexCoords = position;
	vec3 newpos = position * 300.f;
    gl_Position = proj_matrix * model_matrix * vec4(newpos, 1.0);
}
