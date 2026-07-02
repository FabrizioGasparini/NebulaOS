#version 300 es
precision highp float;

uniform sampler2D u_inputTexture;
uniform vec2 u_texelSize;
uniform vec2 u_direction;

in vec2 v_texCoord;
out vec4 fragColor;

void main() {
    /* Optimized 9-tap Gaussian with linear sampling (5 lookups) */
    vec4 result = texture(u_inputTexture, v_texCoord) * 0.227027;

    vec2 offset1 = u_direction * u_texelSize * 1.384615;
    vec2 offset2 = u_direction * u_texelSize * 3.230769;

    result += texture(u_inputTexture, v_texCoord + offset1) * 0.1945946;
    result += texture(u_inputTexture, v_texCoord - offset1) * 0.1945946;
    result += texture(u_inputTexture, v_texCoord + offset2) * 0.054054;
    result += texture(u_inputTexture, v_texCoord - offset2) * 0.054054;

    fragColor = result;
}
