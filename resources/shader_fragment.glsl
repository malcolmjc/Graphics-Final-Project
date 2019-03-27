#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

uniform float floor;
uniform float highlighted;

void main()
{
	vec3 n = normalize(vertex_normal);
    vec3 lp = vec3(0, -200, 0);
    vec3 ld = normalize(vertex_pos - lp);
    float diffuse = dot(n, ld);
    
    vec3 cd = normalize(vertex_pos - campos);
    vec3 h = normalize(cd + ld);
    float spec = dot(n, h);
    spec = clamp(spec, 0, 1);
    spec = pow(spec, 2);

    color = texture(tex, vertex_tex);
	if (highlighted == 1.0)
		color.rgb = vec3(1.0, 251.0/255.0, 204.0/255.0);
    color.rgb += spec * vec3(1.0, 1.0, 1.0);
}
