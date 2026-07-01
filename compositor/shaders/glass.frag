#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texcoord;
uniform sampler2D tex;
uniform vec2 tex_size;
uniform float opacity;
uniform float tint_r;
uniform float tint_g;
uniform float tint_b;
uniform float blur_strength;
uniform float brightness;
uniform float saturation;

mat4 brightnessMatrix(float b) {
    float value = b - 1.0;
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        value, value, value, 1.0
    );
}

mat4 saturationMatrix(float s) {
    vec3 luminance = vec3(0.2126, 0.7152, 0.0722);
    vec3 v = luminance * (1.0 - s);
    return mat4(
        v.x + s, v.x,     v.x,     0.0,
        v.y,     v.y + s, v.y,     0.0,
        v.z,     v.z,     v.z + s, 0.0,
        0.0,     0.0,     0.0,     1.0
    );
}

vec4 gaussian_blur(sampler2D image, vec2 uv, vec2 resolution, float radius) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.3846153846) * radius / resolution;
    vec2 off2 = vec2(3.2307692308) * radius / resolution;

    color += texture2D(image, uv) * 0.2270270270;
    color += texture2D(image, uv + vec2(off1.x, 0.0)) * 0.3162162162;
    color += texture2D(image, uv - vec2(off1.x, 0.0)) * 0.3162162162;
    color += texture2D(image, uv + vec2(0.0, off1.y)) * 0.3162162162;
    color += texture2D(image, uv - vec2(0.0, off1.y)) * 0.3162162162;
    color += texture2D(image, uv + off2) * 0.0702702703;
    color += texture2D(image, uv - off2) * 0.0702702703;
    color += texture2D(image, uv + vec2(off2.x, -off2.y)) * 0.0702702703;
    color += texture2D(image, uv + vec2(-off2.x, off2.y)) * 0.0702702703;

    return color;
}

void main() {
    vec2 uv = v_texcoord;
    vec2 pixel_size = 1.0 / tex_size;

    vec4 blurred = gaussian_blur(tex, uv, tex_size, blur_strength);

    vec4 tinted = vec4(
        blurred.r * tint_r,
        blurred.g * tint_g,
        blurred.b * tint_b,
        blurred.a
    );

    tinted = brightnessMatrix(brightness) * tinted;
    tinted = saturationMatrix(saturation) * tinted;

    tinted.a = opacity;

    gl_FragColor = tinted;
}
