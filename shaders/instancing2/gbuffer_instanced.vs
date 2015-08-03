#version 330 core

uniform mat4 mvp; //modelviewprojection matrix
uniform mat4 normal_mat;

layout(location=0) in vec4 in_vertex; //cube vertex position
layout(location=2) in vec3 in_normal; //cube normal
layout(location=3) in vec4 pos; //instance data

out vec3 normal;

void main()
{
  normal = (normal_mat * vec4(in_normal,0)).xyz;
  gl_Position = mvp * vec4(in_vertex.xyz + pos.xyz, 1);
}
