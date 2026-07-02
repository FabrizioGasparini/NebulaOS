#version 300 es
precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_background;
uniform sampler2D u_blurred;
uniform vec2 u_resolution;
uniform vec2 u_windowSize;
uniform float u_cornerRadius;
uniform float u_blurAmount;
uniform float u_opacity;
uniform vec4 u_tint;
uniform float u_noiseStrength;

out vec4 fragColor;

float sdRoundedRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + vec2(r);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 1689.1984);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    /* SDF mask for rounded corners */
    vec2 fragPos = v_texCoord * u_windowSize;
    vec2 halfSize = u_windowSize * 0.5;
    float dist = sdRoundedRect(fragPos - halfSize, halfSize, u_cornerRadius);
    float mask = 1.0 - smoothstep(-1.0, 1.0, dist);

    if (mask < 0.001) discard;

    /* Sample sharp and blurred backgrounds */
    vec3 sharp = texture(u_background, v_texCoord).rgb;
    vec3 blurred = texture(u_blurred, v_texCoord).rgb;

    /* Blend based on blur amount */
    vec3 color = mix(sharp, blurred, u_blurAmount);

    /* Apply violet/purple tint */
    color *= u_tint.rgb;

    /* Add noise grain for frosted texture */
    float grain = (hash(v_texCoord * u_resolution * 0.5) - 0.5) * u_noiseStrength;
    color += grain;

    /* Per-pixel alpha */
    float alpha = mask * u_opacity;

    fragColor = vec4(color, alpha);
}
