#version 430 core

uniform mat4 model_matrix;
uniform mat4 proj_matrix;
uniform float scale;
uniform vec3 inputPos;
uniform vec3 inputRot;
uniform vec3 inputConst;

uniform mat4 myMatrix;
uniform mat4 instanceMatrix;
uniform vec3 center;

layout(location = 0) in vec3 position;
out vec2 textureCoorVs;

vec3 rotationTransfer(float x, float y, float z,float sX,float sY,float sZ);

void main(void)
{	
	textureCoorVs = position.xy;
	if (textureCoorVs.x < 0) textureCoorVs.x = -textureCoorVs.x;
	if (textureCoorVs.y < 0) textureCoorVs.y = -textureCoorVs.y;
	vec3 newpos = rotationTransfer(position.x, position.y, position.z,
								inputRot.x, inputRot.y, inputRot.z);
								

	newpos = (newpos + inputPos - inputConst)* scale ;
    gl_Position = proj_matrix * model_matrix * (instanceMatrix * vec4(newpos, 1.0) + vec4(center,0));
	// normals = normal;
}

vec3 rotationTransfer(float x, float y, float z,float sX,float sY,float sZ)
{
	float PI = 3.1415926;
	float temp_x = x * (cos(sY* PI / 180.0)*cos(sZ* PI / 180.0) - sin(sX* PI / 180.0)*sin(sY* PI / 180.0)*sin(sZ* PI / 180.0)) + y * (-cos(sX* PI / 180.0)*sin(sZ* PI / 180.0)) + z * (sin(sY* PI / 180.0)*cos(sZ* PI / 180.0) + sin(sX* PI / 180.0)*cos(sY* PI / 180.0)*sin(sZ* PI / 180.0));
	float temp_y = x * (cos(sY* PI / 180.0)*sin(sZ* PI / 180.0) + sin(sX* PI / 180.0)*sin(sY* PI / 180.0)*cos(sZ* PI / 180.0)) + y * (cos(sX* PI / 180.0)*cos(sZ* PI / 180.0)) + z * (sin(sY* PI / 180.0)*sin(sZ* PI / 180.0) - sin(sX* PI / 180.0)*cos(sY* PI / 180.0)*cos(sZ* PI / 180.0));
	float temp_z = x * (-cos(sX* PI / 180.0)*sin(sY* PI / 180.0)) + y * (sin(sX* PI / 180.0)) + z * (cos(sX* PI / 180.0)*cos(sY* PI / 180.0));
	return vec3(temp_x, temp_y, temp_z);
}


mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}