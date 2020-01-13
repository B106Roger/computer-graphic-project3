#version 430 core

// 聲明input primitive類型
layout(triangles) in;
// 聲明output primitive類型，output頂點數量多於max_vertices會被忽略
layout(triangle_strip, max_vertices=9) out;

// in vec3 normals[];

out vec3 vs_worldpos;
out vec3 vs_normal;

out vec2 texture_coord;
out vec3 normalVect;
out vec3 lightVect;
out vec3 eyeVect;
out vec3 halfWayVect;
out vec3 reflectVect;

in vec2 i_texture_coord;
in vec3 i_normalVect;
in vec3 i_lightVect;
in vec3 i_eyeVect;
in vec3 i_halfWayVect;
in vec3 i_reflectVect;


void main( void )
{
	vec3 a = ( gl_in[1].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 b = ( gl_in[2].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 N = normalize( cross(  a,b ) );
	
    for( int i=0; i<gl_in.length( ); ++i )
    {
        gl_Position = gl_in[i].gl_Position;
		vs_worldpos = gl_in[i].gl_Position.xyz;
		vs_normal = N;

        texture_coord = i_texture_coord;
        normalVect = i_normalVect;
        lightVect = i_lightVect;
        eyeVect = i_eyeVect;
        halfWayVect = i_halfWayVect;
        reflectVect = i_reflectVect;

        EmitVertex( );
    }

    EndPrimitive( );
}