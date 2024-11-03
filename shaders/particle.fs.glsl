#version 330 core

in vec2 tex_coord;
out vec4 color;

uniform sampler2D tex;
uniform vec4 texture_color;

void main() {
    vec4 tex_sample = texture(tex, tex_coord);
    color = tex_sample * texture_color;
}