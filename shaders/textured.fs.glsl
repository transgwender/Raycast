#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform bool highlight;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec4 baseColor = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

    if (highlight) {
        baseColor.rgb += vec3(0.18);
    }

    color = baseColor;
}
