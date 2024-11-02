#version 330 core

// Input attributes
layout (location = 0) in vec4 vertex;

// Passed to fragment shader
out vec2 tex_coord;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main() {
	vec3 pos = projection * transform * vec3(vertex.xy, 0.0);
	gl_Position = vec4(pos.xy, 0.0, 1.0);
	tex_coord = vertex.zw;
}
