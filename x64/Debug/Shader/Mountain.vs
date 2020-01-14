#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform vec3 centerPosition;
uniform int mountain_length;
uniform int mountain_width;

out vec2 textureCoord;
layout(location = 0) in vec3 position;

void main(void)
{
	float wave = 3.f;
	vec3 newPos = position;
	newPos.x -= centerPosition.x;
	newPos.z -= centerPosition.z;
	
    gl_Position = proj_matrix * model_matrix * vec4(newPos, 1.0);
	
	/*if (newPos.x > 0 && newPos.z > 0)
	{
		textureCoord.x = newPos.x / (mountain_width / 2.f);
		textureCoord.y = newPos.z / (mountain_length / 2.f);
	}
	else if (newPos.x > 0 && newPos.z < 0)
	{
		textureCoord.x = newPos.x / (mountain_width / 2.f);
		textureCoord.y = -newPos.z / (mountain_length / 2.f);
	}
	else if (newPos.x < 0 && newPos.z > 0)
	{
		textureCoord.x = -newPos.x / (mountain_width / 2.f);
		textureCoord.y = newPos.z / (mountain_length / 2.f);
	}
	else if (newPos.x < 0 && newPos.z < 0)
	{
		textureCoord.x = -newPos.x / (mountain_width / 2.f);
		textureCoord.y = -newPos.z / (mountain_length / 2.f);
	}
	if (textureCoord.x < 0.00001)
		textureCoord.x = 0;
	if (textureCoord.y < 0.00001)
		textureCoord.y = 0;*/
	textureCoord.x = position.x / (mountain_width / 10.f);
	textureCoord.y = position.z / (mountain_length / 10.f);
	
}
