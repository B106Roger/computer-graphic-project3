#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float scale;
uniform vec3 inputPos;
uniform vec3 inputRot;
uniform vec3 inputConst;
uniform mat3 myMatrix;
uniform float angle;
out int newColorInput;
out int reverseNormal;
layout(location = 0) in vec3 position;

void main(void)
{	
	float myangle = atan(position.x,position.y)/3.1415926 * 180;
	if (position.z > 0)
		reverseNormal = 1;
	else 
		reverseNormal = 0;
		

	if (abs(myangle-angle) < 30 || abs(myangle-angle+360) < 30)
		newColorInput = 0;
	else
		newColorInput = 1;
		

	mat3 inverse_matrix = inverse(myMatrix);
	vec3 newpos = inverse_matrix * position;
	newpos = (newpos + inputPos - inputConst); // * scale 
    gl_Position = proj_matrix * model_matrix * vec4(newpos, 1.0);
}
