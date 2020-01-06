#version 430 core

layout (location = 0) out vec4 color;

uniform mat4 model_matrix;
uniform mat4 proj_matrix;

in vec3 vs_worldpos;
in vec3 vs_normal;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
	vec3 Normal = mat3(transpose(inverse(model_matrix))) * vs_normal;
	vec3 Position = vec3(model_matrix * vec4(vs_worldpos, 1.0));
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	color = vec4(texture(skybox, R).rgb, 1.0);
}
