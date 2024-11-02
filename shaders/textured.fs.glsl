/**
Shader for drawing textured quads.

Lighting adapted from https://learnopengl.com/Lighting/Basic-Lighting and https://learnopengl.com/Lighting/Multiple-lights
specifically the point light calculations
*/
#version 330

#define MAX_LIGHTS 128

struct PointLight {
    vec3 position;
    vec3 diffuse;

    float constant;
    float linear;
    float quadratic;
};

in vec2 tex_coord;
in vec3 frag_pos;

uniform sampler2D albedo_tex;
uniform sampler2D normal_tex;
uniform bool highlight;

uniform vec3 ambient_light;

uniform int point_lights_count;
uniform PointLight point_lights[MAX_LIGHTS];

uniform bool is_spritesheet;
uniform float horizontal_offset;
uniform float vertical_offset;
uniform vec2 cell_size;

layout(location = 0) out vec4 color;

vec3 calculate_ambient_light() {
    vec3 albedo = vec3(texture(albedo_tex, tex_coord));
    return ambient_light * albedo;
}

// Calculate the diffuse value of a fragment after a point light has been applied onto it.
vec3 calculate_point_light(PointLight light) {
    vec3 light_dir = normalize(light.position - frag_pos);

    // diffuse shading
	vec3 normal = normalize((texture(normal_tex, tex_coord).xyz * 2.0 - 1.0)*0.25);
    float diff = max(dot(normal, light_dir), 0.0);

    // attenuation
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)));

    // calculate final light
    vec3 albedo = vec3(texture(albedo_tex, tex_coord));
    vec3 diffuse_component = light.diffuse * diff * albedo;
    return diffuse_component*attenuation;
}

void main() {
    vec3 result = calculate_ambient_light();
    for (int i = 0; i < point_lights_count; i++) {
        result += calculate_point_light(point_lights[i]);
    }

    if (highlight) {
        result += vec3(0.18);
    }

    color = vec4(result, (texture(albedo_tex, tex_coord)).w);
}
