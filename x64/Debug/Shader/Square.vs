#version 430 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 uv;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform int t;
out vec2 vUV;

void main(void)
{
	vec3 v = vec3(vertex.x, sqrt(vertex.x*vertex.x+vertex.z*vertex.z), vertex.z);
    gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(v, 1.0);
    vUV = uv;
}
