#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D walltex;

void main()
{
	vec4 tcol = texture(walltex, vertex_tex);
	color = tcol;
}
