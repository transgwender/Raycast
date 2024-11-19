#version 330 core

in vec2 tex_coord;
in vec4 fcolor;

out vec4 color;

uniform sampler2D tex;

void main() {
    vec4 tex_sample = texture(tex, tex_coord);
    color = tex_sample * fcolor;
}