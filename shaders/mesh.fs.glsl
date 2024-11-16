#version 330

// From Vertex Shader
in vec3 vcolor;
in vec2 vpos; // Distance from local origin

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;
uniform bool is_minisun;
uniform float light_level;

// Output color
layout(location = 0) out vec4 color;

void main() {
	color = vec4(fcolor * vcolor, 1.0);
	float radius = distance(vec2(0.0), vpos);
	if (light_up == 1)
	{
		// 0.8 is just to make it not too strong
		color.xyz += vec3(1.0, 1.0, 0.0);
	}

	if (is_minisun)
	{
		// 0.8 is just to make it not too strong
		color.xyz += vec3(light_level * 1.0, light_level * 1.0, 0.0);
	}

}
