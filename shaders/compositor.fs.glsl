#version 330

uniform sampler2D world;
uniform sampler2D ui;
uniform sampler2D world_text;
uniform sampler2D ui_text;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
    vec4 world_color = texture(world, texcoord);
    vec4 ui_color = texture(ui, texcoord);
    vec4 world_text_color = texture(world_text, texcoord);
    vec4 ui_text_color = texture(ui_text, texcoord);

    color = mix(world_color, world_text_color, world_text_color.a);
    color = mix(color, ui_color, ui_color.a);
    color = mix(color, ui_text_color, ui_text_color.a);
}