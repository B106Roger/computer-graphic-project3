#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;

layout(location = 0) in vec3 position;
out vec3 TexCoords;


void main(void)
{	
	TexCoords = position;
	vec3 newpos = position * 10.f;
	
	mat4 new_model_matrix = mat4(mat3(model_matrix));
	
    gl_Position = proj_matrix * new_model_matrix * vec4(newpos, 1.0);
}
