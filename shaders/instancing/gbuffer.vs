#version 330 core

uniform mat4 mvp;
uniform mat4 normal_mat;
uniform vec4 pos;

layout(location=0) in vec4 in_vertex;
layout(location=2) in vec3 in_normal;

out vec3 normal;

void main()
{
  normal = (normal_mat * vec4(in_normal,0)).xyz;
  gl_Position = mvp * vec4(in_vertex.xyz + pos.xyz, 1);
}
