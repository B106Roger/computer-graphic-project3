#version 430 core

// 聲明input primitive類型
layout(triangles) in;
// 聲明output primitive類型，output頂點數量多於max_vertices會被忽略
layout(triangle_strip, max_vertices=6) out;

out vec3 vs_worldpos;
out vec3 vs_normal;



void main( void )
{
    vec3 a = ( gl_in[1].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 b = ( gl_in[2].gl_Position - gl_in[0].gl_Position ).xyz;
    vec3 N = normalize( cross( b, a ) );

    for( int i=0; i<gl_in.length( ); ++i )
    {
        gl_Position = gl_in[i].gl_Position;
        vs_normal = N;
		vs_worldpos = gl_in[i].gl_Position.xyz;
        EmitVertex( );
    }

    EndPrimitive( );
}