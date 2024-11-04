#version 330

uniform sampler2D sprite_stage_tex;
uniform sampler2D mesh_stage_tex;
uniform sampler2D particle_stage_tex;
uniform sampler2D menu_stage_tex;
uniform sampler2D text_stage_tex;

uniform float time;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
    vec4 sprite_color = texture(sprite_stage_tex, texcoord);
    vec4 mesh_color = texture(mesh_stage_tex, texcoord);
    vec4 menu_color = texture(menu_stage_tex, texcoord);
    vec4 particle_color = texture(particle_stage_tex, texcoord);
    vec4 text_color = texture(text_stage_tex, texcoord);

    color = mix(sprite_color, mesh_color, mesh_color.a);
    color = mix(color, particle_color, particle_color.a);
    color = mix(color, menu_color, menu_color.a);
    color = mix(color, text_color, text_color.a);
}