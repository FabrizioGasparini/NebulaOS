#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texcoord;
uniform sampler2D tex;
uniform vec2 tex_size;
uniform float blur_radius;
uniform int num_passes;

vec4 blur_h(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.3846153846) * direction;
    vec2 off2 = vec2(3.2307692308) * direction;
    color += texture2D(image, uv) * 0.2270270270;
    color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;
    color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;
    color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;
    color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;
    return color;
}

void main() {
    vec2 uv = v_texcoord;
    vec2 pixel_size = 1.0 / tex_size;

    vec4 color = texture2D(tex, uv);

    if (blur_radius > 0.0) {
        for (int i = 0; i < 4; i++) {
            float offset = float(i + 1) * blur_radius * pixel_size.x;
            color += texture2D(tex, uv + vec2(offset, 0.0));
            color += texture2D(tex, uv - vec2(offset, 0.0));
            color += texture2D(tex, uv + vec2(0.0, offset));
            color += texture2D(tex, uv - vec2(0.0, offset));
        }
        color /= (1.0 + 4.0 * 4.0);
    }

    gl_FragColor = color;
}
