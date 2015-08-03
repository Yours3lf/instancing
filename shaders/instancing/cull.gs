#version 330 core

layout( points ) in;
layout( points, max_vertices = 1 ) out;

in vec4 pos_copy[1];
flat in int visible[1];

out vec4 culled_pos;

void main()
{
  if( bool(visible[0]) )
  {
		culled_pos = pos_copy[0];
		EmitVertex();
		EndPrimitive();
  }
}
