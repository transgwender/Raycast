/**
 * Post processing shader.
 * Bloom shader code loosely adapted from https://learnopengl.com/Advanced-Lighting/Bloom
 * Many things were changed, mainly having the brightness pass and compositing stage done in the same shader.
 * Also, instead of using more render passes, I used a larger Gaussian kernel with fewer passes.
 */

#version 330

uniform sampler2D input1;
uniform sampler2D input2;

in vec2 texcoord;

layout(location = 0) out vec4 color;

// NOTE: this is not a full Gaussian kernel, but rather half of one. This is because we symmetrically
// add/multiply 2 pixels in each loop iteration when convolving this kernel with the image.
// thus, as a full 1d Gaussian kernel, this would actually be of size 9
#define KERNEL_SIZE 5
uniform float kernel[KERNEL_SIZE] = float[](0.20416369, 0.18017382, 0.12383154, 0.06628225, 0.02763055);

uniform vec2 screen_size;

// shader mode, 0 = brightness pass, 1 = horizontal blur, 2 = vertical blur, 3 = composite
uniform int mode;

// calculate the brightness (luminance) of the given rgb pixel
float brightness(vec3 color) {
    return (0.2126 * color.r) + (0.7152 * color.g) + (0.0722 * color.b);
}

void main() {
    if (mode == 0) {
        vec4 tex_color = texture(input1, texcoord);
        if (brightness(tex_color.rgb) <= 1.5) {
            color = vec4(0.0);
        } else {
            color = tex_color;
        }
        return;
    }

    if (mode == 3) {
        vec3 blurred = texture(input1, texcoord).rgb;
        vec3 scene = texture(input2, texcoord).rgb;
        scene += blurred;

        color = vec4(scene, 1.0);
        return;
    }

    // else, apply blur
    vec2 texel_size = vec2(1.0) / screen_size;
    vec3 result = texture(input1, texcoord).rgb * kernel[0];

    for (int i = 1; i < KERNEL_SIZE; i++) {
        // as mentioned above, symmetrically apply the kernel to two pixels on either size of the central pixel, offset based on the index
        if (mode == 1) { // horizontal
            result += texture(input1, texcoord + vec2(i * texel_size.x, 0.0)).rgb * kernel[i];
            result += texture(input1, texcoord - vec2(i * texel_size.x, 0.0)).rgb * kernel[i];
        } else if (mode == 2) { // vertical
            result += texture(input1, texcoord + vec2(0.0, i * texel_size.y)).rgb * kernel[i];
            result += texture(input1, texcoord - vec2(0.0, i * texel_size.y)).rgb * kernel[i];
        }
    }

    color = vec4(result, 1.0);
}