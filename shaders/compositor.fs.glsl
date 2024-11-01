#version 330

uniform sampler2D sprite_stage_tex;
uniform sampler2D text_stage_tex;

uniform float time;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
    color = texture(sprite_stage_tex, texcoord) + texture(text_stage_tex, texcoord);
}