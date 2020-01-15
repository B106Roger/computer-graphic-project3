#version 430 core

layout (location = 0) out vec4 color;

in vec3 vs_worldpos;
in vec3 vs_normal;

uniform vec4 color_ambient = vec4(0.1,0.2,0.5,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform vec4 Color = vec4(0.1,0.1,0.1,0.1);
uniform float shininess = 77.0f;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 EyeDirection = vec3(0.0,0.0,1.0);

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform sampler2D textures;
uniform samplerCube skybox;
uniform vec3 cameraPos;
in vec2 textureCoorGs;


void main()
{
	//vec3 light_direction = normalize(light_position - vs_worldpos);
	//vec3 normal = normalize(vs_normal);
	//vec3 half_vector = normalize(normalize(light_direction) + normalize(EyeDirection));
	
	//float diffuse = max(0.0, dot(normal, light_direction));
	//float specular = pow(max(0.0, dot(vs_normal, half_vector)), shininess);
	//color = min(Color * color_ambient, vec4(1.0))
	//		+ diffuse * color_diffuse + specular * color_specular;
	vec3 Normal = mat3(transpose(inverse(model_matrix))) * vs_normal;
	vec3 Position = vec3(model_matrix * vec4(vs_worldpos, 1.0));
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	vec4 reflectionColor = vec4(texture(skybox, R).rgb, 1.0);;
	vec4 textureColor = vec4(texture(textures,textureCoorGs).rgb,1);
	color = textureColor  * 0.5 + reflectionColor * 0.5;
	color.w = 1;
}
