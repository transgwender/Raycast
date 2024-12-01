#version 330

uniform sampler2D screen;
in vec2 texcoord;

layout(location = 0) out vec4 color;

void main() {
    color = texture(screen, texcoord);
}