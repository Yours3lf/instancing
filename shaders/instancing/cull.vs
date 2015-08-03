#version 330 core

uniform mat4 mvp;

layout(location = 0) in vec4 pos;

out vec4 pos_copy;
flat out int visible;

void main()
{
  pos_copy = pos;
  
  vec4 bb[8];
  bb[0] = mvp * ( pos + vec4(  2,  2,  2, 0 ) );
  bb[1] = mvp * ( pos + vec4( -2,  2,  2, 0 ) );
  bb[2] = mvp * ( pos + vec4(  2, -2,  2, 0 ) );
  bb[3] = mvp * ( pos + vec4( -2, -2,  2, 0 ) );
  bb[4] = mvp * ( pos + vec4(  2,  2, -2, 0 ) );
  bb[5] = mvp * ( pos + vec4( -2,  2, -2, 0 ) );
  bb[6] = mvp * ( pos + vec4(  2, -2, -2, 0 ) );
  bb[7] = mvp * ( pos + vec4( -2, -2, -2, 0 ) );
  
  int oob[6] = int[6]( 0, 0, 0, 0, 0, 0 );
  
  for( int c = 0; c < 8; ++c )
  {
		oob[0] += int( bb[c].x >  bb[c].w );
		oob[1] += int( bb[c].x < -bb[c].w );
		oob[2] += int( bb[c].y >  bb[c].w );
		oob[3] += int( bb[c].y < -bb[c].w );
		oob[4] += int( bb[c].z >  bb[c].w );
		oob[5] += int( bb[c].z < -bb[c].w );
  }
  
  bool in_frustum = true;
  
  for( int c = 0; c < 6; ++c )
  {
    in_frustum = in_frustum && oob[c] != 8;
  }
  
  visible = int( in_frustum );
}