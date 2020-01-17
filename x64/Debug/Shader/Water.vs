#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float time;

// custom water height map
float waterHeight = 3.0;
float pi = 3.1415926;
int numWaves = 4;
float amplitude[8];
float wavelength[8];
float speed[8];
vec2 direction[8];


layout(location = 0) in vec2 position;
out vec2 textureCoorVs;

float wave(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    return amplitude[i] * sin(theta * frequency + time * phase);
}

float waveHeight(float x, float y) {
    float height = 0.0;
    for (int i = 0; i < numWaves; ++i)
        height += wave(i, x, y);
    return height;
}

float dWavedx(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].x * frequency;
    return A * cos(theta * frequency + time * phase);
}

float dWavedy(int i, float x, float y) {
    float frequency = 2*pi/wavelength[i];
    float phase = speed[i] * frequency;
    float theta = dot(direction[i], vec2(x, y));
    float A = amplitude[i] * direction[i].y * frequency;
    return A * cos(theta * frequency + time * phase);
}

vec3 waveNormal(float x, float y) {
    float dx = 0.0;
    float dy = 0.0;
    for (int i = 0; i < numWaves; ++i) {
        dx += dWavedx(i, x, y);
        dy += dWavedy(i, x, y);
    }
    vec3 n = vec3(-dx, -dy, 1.0);
    return normalize(n);
}

void main(void)
{
	vec3 displacement = vec3(50,5,30);
	textureCoorVs = position.xy + vec2(100,100);
	textureCoorVs = textureCoorVs / 200;
	
	// wave para
	float M_PI = 3.1415926;
	for (int i = 0; i < 4; ++i) {
        amplitude[i] = 0.5f / (i + 1);
		
        wavelength[i] = 8 * M_PI / (i + 1);
        
        speed[i] = 1.0f + 2*i;
        
		float tmp = fract(sin(dot(position.xy ,vec2(12.9898,78.233))) * 432347.5453);

        float angle = tmp / 10000;
		
		direction[i] = vec2(cos(angle), sin(angle));
    }
	
	//float wave = 3.f;
	float distance= sqrt(position.x * position.x + position.y * position.y);
	// float height = cos(1000.f - distance / 4.f + time) * wave;
	float height = waveHeight(position.x, position.y);
	vec3 new_position = vec3(
	position.x+displacement.x,
	height+displacement.y, 
	position.y+displacement.z);
	
    gl_Position = proj_matrix * model_matrix * vec4(new_position, 1.0);
}


