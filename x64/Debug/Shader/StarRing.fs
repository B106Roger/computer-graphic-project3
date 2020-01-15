#version 430 core

layout (location = 0) out vec4 color;
uniform sampler2D mountain_texture;
in vec3 vs_worldpos;
in vec3 vs_normal;

uniform vec4 color_ambient = vec4(0.4,0.2,0.4,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);

uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform vec4 Color = vec4(0.5,0.1,0.1,0.1);

uniform float shininess = 77.0f;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 EyeDirection = vec3(0.0,0.0,1.0);
in vec2 textureCoorGs;

void main()
{
	vec3 light_direction = normalize(light_position - vs_worldpos);
	vec3 normal = normalize(vs_normal);
	vec3 half_vector = normalize(normalize(light_direction) + normalize(EyeDirection));
	
	float diffuse = max(0.0, dot(normal, light_direction));
	float specular = pow(max(0.0, dot(vs_normal, half_vector)), shininess);
	color = texture(mountain_texture,textureCoorGs);//min(Color * color_ambient, vec4(1.0))
			//+ diffuse * color_diffuse + specular * color_specular;

}
