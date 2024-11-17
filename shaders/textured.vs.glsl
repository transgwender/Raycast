#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 tex_coord;
out vec3 frag_pos;

// Application data
uniform mat3 transform;
uniform mat3 projection;

// Spritesheet data
uniform float horizontal_offset;
uniform float vertical_offset;
uniform vec2 cell_size;

void main() {
	tex_coord = (cell_size * in_texcoord) + vec2(horizontal_offset, vertical_offset);
	frag_pos = transform * vec3(in_position.xy, 1.0);
	frag_pos.z = in_position.z;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}