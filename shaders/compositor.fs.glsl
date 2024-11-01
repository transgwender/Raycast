#version 330

uniform sampler2D sprite_stage_tex;
uniform sampler2D text_stage_tex;

uniform float time;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
    vec4 sprite_color = texture(sprite_stage_tex, texcoord);
    vec4 text_color = texture(text_stage_tex, texcoord);
    color = mix(sprite_color, text_color, text_color.a);
}