#version 330 core
in vec2 TexCoords;

layout(location = 0) out vec4 world_color;
layout(location = 1) out vec4 ui_color;

uniform sampler2D text;
uniform vec3 textColor;

uniform int layer;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);

    if (layer == 3) {
        world_color = vec4(textColor, 1.0) * sampled;
        ui_color = vec4(0.0);
    } else if (layer == 6) {
        ui_color = vec4(textColor, 1.0) * sampled;
        world_color = vec4(0.0);
    } else {
        ui_color = vec4(0.0);
        world_color = vec4(0.0);
    }
}