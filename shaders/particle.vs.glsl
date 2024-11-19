#version 330 core

// Input attributes
layout (location = 0) in vec2 in_pos;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 2) in vec2 world_pos;
layout (location = 3) in vec2 scale;
layout (location = 4) in vec4 in_color;
layout (location = 5) in float angle;

// Passed to fragment shader
out vec2 tex_coord;
out vec4 fcolor;

// Application data
uniform mat3 projection;

mat3 create_transform_matrix() {
    float x = world_pos.x;
    float y = world_pos.y;
    mat3 translate_mat;
    translate_mat[0] = vec3(1, 0, 0);
    translate_mat[1] = vec3(0, 1, 0);
    translate_mat[2] = vec3(x, y, 1);

    float c = cos(angle);
    float s = sin(angle);
    mat3 rotate_mat;
    rotate_mat[0] = vec3(c, s, 0);
    rotate_mat[1] = vec3(-s, c, 0);
    rotate_mat[2] = vec3(0, 0, 1);

    float sx = scale.x;
    float sy = scale.y;
    mat3 scale_mat;
    scale_mat[0] = vec3(sx, 0, 0);
    scale_mat[1] = vec3(0, sy, 0);
    scale_mat[2] = vec3(0, 0, 1);

    return translate_mat * rotate_mat * scale_mat;
}

void main() {
    mat3 transform = create_transform_matrix();
	vec3 pos = projection * transform * vec3(in_pos, 1.0);

	gl_Position = vec4(pos.xy, 0.0, 1.0);
	tex_coord = in_tex_coord;
	fcolor = in_color;
}
