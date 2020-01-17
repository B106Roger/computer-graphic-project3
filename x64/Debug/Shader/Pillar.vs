#version 430 core
uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform vec3 inputLoc;
layout(location = 0) in vec3 position;

void main(void)
{
	vec3 newpos = vec3(
	position.x * 0.5 + inputLoc.x,
	0, 
	position.z * 0.5 + inputLoc.z
	);
	if (position.y == 1)
	{
		newpos.y = inputLoc.y;
	}
	
    gl_Position = proj_matrix * model_matrix * vec4(newpos, 1.0);
}