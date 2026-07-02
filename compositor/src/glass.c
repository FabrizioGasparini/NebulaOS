#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <GLES2/gl2.h>
#include <wlr/util/log.h>

#include "nebula/glass.h"

static const char *blur_vert_src =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 a_position;\n"
    "in vec2 a_texCoord;\n"
    "out vec2 v_texCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
    "    v_texCoord = a_texCoord;\n"
    "}\n";

static const char *blur_frag_src =
    "#version 300 es\n"
    "precision highp float;\n"
    "uniform sampler2D u_inputTexture;\n"
    "uniform vec2 u_texelSize;\n"
    "uniform vec2 u_direction;\n"
    "in vec2 v_texCoord;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "    vec4 result = texture(u_inputTexture, v_texCoord) * 0.227027;\n"
    "    vec2 offset1 = u_direction * u_texelSize * 1.384615;\n"
    "    vec2 offset2 = u_direction * u_texelSize * 3.230769;\n"
    "    result += texture(u_inputTexture, v_texCoord + offset1) * 0.1945946;\n"
    "    result += texture(u_inputTexture, v_texCoord - offset1) * 0.1945946;\n"
    "    result += texture(u_inputTexture, v_texCoord + offset2) * 0.054054;\n"
    "    result += texture(u_inputTexture, v_texCoord - offset2) * 0.054054;\n"
    "    fragColor = result;\n"
    "}\n";

static const char *glass_vert_src =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 a_position;\n"
    "in vec2 a_texCoord;\n"
    "out vec2 v_texCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
    "    v_texCoord = a_texCoord;\n"
    "}\n";

static const char *glass_frag_src =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 v_texCoord;\n"
    "uniform sampler2D u_background;\n"
    "uniform sampler2D u_blurred;\n"
    "uniform vec2 u_resolution;\n"
    "uniform vec2 u_windowSize;\n"
    "uniform float u_cornerRadius;\n"
    "uniform float u_blurAmount;\n"
    "uniform float u_opacity;\n"
    "uniform vec4 u_tint;\n"
    "uniform float u_noiseStrength;\n"
    "out vec4 fragColor;\n"
    "float sdRoundedRect(vec2 p, vec2 b, float r) {\n"
    "    vec2 q = abs(p) - b + vec2(r);\n"
    "    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\n"
    "}\n"
    "float hash(vec2 p) {\n"
    "    vec3 p3 = fract(vec3(p.xyx) * 1689.1984);\n"
    "    p3 += dot(p3, p3.yzx + 33.33);\n"
    "    return fract((p3.x + p3.y) * p3.z);\n"
    "}\n"
    "void main() {\n"
    "    vec2 fragPos = v_texCoord * u_windowSize;\n"
    "    vec2 halfSize = u_windowSize * 0.5;\n"
    "    float dist = sdRoundedRect(fragPos - halfSize, halfSize, u_cornerRadius);\n"
    "    float mask = 1.0 - smoothstep(-1.0, 1.0, dist);\n"
    "    if (mask < 0.001) discard;\n"
    "    vec3 sharp = texture(u_background, v_texCoord).rgb;\n"
    "    vec3 blurred = texture(u_blurred, v_texCoord).rgb;\n"
    "    vec3 color = mix(sharp, blurred, u_blurAmount);\n"
    "    color *= u_tint.rgb;\n"
    "    float grain = (hash(v_texCoord * u_resolution * 0.5) - 0.5) * u_noiseStrength;\n"
    "    color += grain;\n"
    "    float alpha = mask * u_opacity;\n"
    "    fragColor = vec4(color, alpha);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        wlr_log(WLR_ERROR, "Shader compilation failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint create_program(const char *vert_src, const char *frag_src) {
    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);

    if (!vert || !frag) {
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        wlr_log(WLR_ERROR, "Program linking failed: %s", log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

static void setup_quad(struct nebula_glass *glass) {
    static const float quad_vertices[] = {
        /* position    texcoord */
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
    };

    glGenVertexArrays(1, &glass->quad_vao);
    glGenBuffers(1, &glass->quad_vbo);

    glBindVertexArray(glass->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, glass->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

static void create_fbo_pair(struct nebula_glass *glass, int width, int height) {
    for (int i = 0; i < 2; i++) {
        glGenFramebuffers(1, &glass->pingpong_fbo[i]);
        glGenTextures(1, &glass->pingpong_tex[i]);

        glBindTexture(GL_TEXTURE_2D, glass->pingpong_tex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, glass->pingpong_fbo[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, glass->pingpong_tex[i], 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool glass_init(struct nebula_glass *glass, int width, int height) {
    memset(glass, 0, sizeof(*glass));

    glass->blur_program = create_program(blur_vert_src, blur_frag_src);
    glass->glass_program = create_program(glass_vert_src, glass_frag_src);

    if (!glass->blur_program || !glass->glass_program) {
        wlr_log(WLR_ERROR, "Failed to create glass shader programs");
        return false;
    }

    setup_quad(glass);

    glass->blur_width = width / 2;
    glass->blur_height = height / 2;
    create_fbo_pair(glass, glass->blur_width, glass->blur_height);

    /* Default glass parameters */
    glass->blur_amount = 0.8f;
    glass->corner_radius = 16.0f;
    glass->noise_strength = 0.03f;
    glass->tint_r = 0.35f;
    glass->tint_g = 0.15f;
    glass->tint_b = 0.65f;
    glass->tint_a = 0.12f;
    glass->opacity = 0.85f;

    wlr_log(WLR_INFO, "Glass effects initialized (%dx%d)", width, height);
    return true;
}

void glass_destroy(struct nebula_glass *glass) {
    if (glass->blur_program) {
        glDeleteProgram(glass->blur_program);
    }
    if (glass->glass_program) {
        glDeleteProgram(glass->glass_program);
    }
    for (int i = 0; i < 2; i++) {
        if (glass->pingpong_fbo[i]) {
            glDeleteFramebuffers(1, &glass->pingpong_fbo[i]);
        }
        if (glass->pingpong_tex[i]) {
            glDeleteTextures(1, &glass->pingpong_tex[i]);
        }
    }
    if (glass->quad_vao) {
        glDeleteVertexArrays(1, &glass->quad_vao);
    }
    if (glass->quad_vbo) {
        glDeleteBuffers(1, &glass->quad_vbo);
    }
}

void glass_resize(struct nebula_glass *glass, int width, int height) {
    int new_w = width / 2;
    int new_h = height / 2;

    if (new_w == glass->blur_width && new_h == glass->blur_height) {
        return;
    }

    for (int i = 0; i < 2; i++) {
        glDeleteFramebuffers(1, &glass->pingpong_fbo[i]);
        glDeleteTextures(1, &glass->pingpong_tex[i]);
    }

    glass->blur_width = new_w;
    glass->blur_height = new_h;
    create_fbo_pair(glass, new_w, new_h);
}

void glass_render_blur(struct nebula_glass *glass, GLuint input_tex, int iterations) {
    glDisable(GL_BLEND);

    glUseProgram(glass->blur_program);
    glBindVertexArray(glass->quad_vao);

    bool horizontal = true;
    bool first_iteration = true;

    for (int i = 0; i < iterations * 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, glass->pingpong_fbo[horizontal]);
        glViewport(0, 0, glass->blur_width, glass->blur_height);

        glUniform2f(glGetUniformLocation(glass->blur_program, "u_direction"),
            horizontal ? 1.0f : 0.0f,
            horizontal ? 0.0f : 1.0f);
        glUniform2f(glGetUniformLocation(glass->blur_program, "u_texelSize"),
            1.0f / glass->blur_width, 1.0f / glass->blur_height);

        glActiveTexture(GL_TEXTURE0);
        if (first_iteration) {
            glBindTexture(GL_TEXTURE_2D, input_tex);
            first_iteration = false;
        } else {
            glBindTexture(GL_TEXTURE_2D, glass->pingpong_tex[!horizontal]);
        }
        glUniform1i(glGetUniformLocation(glass->blur_program, "u_inputTexture"), 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
}

void glass_render_quad(struct nebula_glass *glass, GLuint blurred_tex,
        GLuint background_tex, float win_w, float win_h) {
    glUseProgram(glass->glass_program);
    glBindVertexArray(glass->quad_vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurred_tex);
    glUniform1i(glGetUniformLocation(glass->glass_program, "u_blurred"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, background_tex);
    glUniform1i(glGetUniformLocation(glass->glass_program, "u_background"), 1);

    glUniform2f(glGetUniformLocation(glass->glass_program, "u_resolution"),
        (float)glass->blur_width * 2, (float)glass->blur_height * 2);
    glUniform2f(glGetUniformLocation(glass->glass_program, "u_windowSize"),
        win_w, win_h);
    glUniform1f(glGetUniformLocation(glass->glass_program, "u_cornerRadius"),
        glass->corner_radius);
    glUniform1f(glGetUniformLocation(glass->glass_program, "u_blurAmount"),
        glass->blur_amount);
    glUniform1f(glGetUniformLocation(glass->glass_program, "u_opacity"),
        glass->opacity);
    glUniform4f(glGetUniformLocation(glass->glass_program, "u_tint"),
        glass->tint_r, glass->tint_g, glass->tint_b, glass->tint_a);
    glUniform1f(glGetUniformLocation(glass->glass_program, "u_noiseStrength"),
        glass->noise_strength);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
}
