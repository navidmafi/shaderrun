#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <errno.h>

char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        errno = ENOENT;
        fprintf(stderr, "[ERR] failed to open file: %s\n", path);
        exit(ENOENT);
    }

    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);

    char *buf = malloc(n + 1);
    fread(buf, 1, n, f);
    buf[n] = 0;

    fclose(f);
    return buf;
}

GLuint compile(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "[SHADER ERROR]\n%s\n", log);
        return 0;
    }
    return s;
}

GLuint link_program(GLuint vs, GLuint fs)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetProgramInfoLog(p, sizeof(log), NULL, log);
        fprintf(stderr, "[LINK ERROR]\n%s\n", log);
        return 0;
    }
    return p;
}

time_t last_mod_time(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return st.st_mtime;
}

char *inject_wrap(const char *fs_src)
{
    const char *prefix =
        "#version 330 core\n"
        "uniform vec2 iResolution;\n"
        "uniform float iTime;\n\n";

    const char *suffix =
        "\nvoid main() {\n"
        "    vec4 fragColor;\n"
        "    mainImage(fragColor, gl_FragCoord.xy);\n"
        "    gl_FragColor = fragColor;\n"
        "}\n";

    size_t len = strlen(prefix) + strlen(fs_src) + strlen(suffix) + 1;
    char *buf = malloc(len);
    strcpy(buf, prefix);
    strcat(buf, fs_src);
    strcat(buf, suffix);
    return buf;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s shader.frag\n", argv[0]);
        return 1;
    }
    const char *frag_path = argv[1];

    if (!glfwInit())
    {
        fprintf(stderr, "[ERR] glfwInit failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(600, 800, "shader", NULL, NULL);
    if (!window)
        return 1;
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        fprintf(stderr, "[ERR] failed to initialize OpenGL\n");
        return 1;
    }

    const char *vs_src =
        "#version 330 core\n"
        "layout(location=0) in vec2 pos;\n"
        "void main(){ gl_Position = vec4(pos,0,1); }\n";

    GLuint prog = 0;
    time_t last_time = 0;

    float quad[] = {-1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1};
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    double t0 = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // Check shader file modification
        time_t mod_time = last_mod_time(frag_path);
        if (mod_time != last_time)
        {
            last_time = mod_time;
            char *fs_src_raw = read_file(frag_path);
            if (!fs_src_raw)
            {
                fprintf(stderr, "Shader file nonexistent");
            }
            char *fs_src = inject_wrap(fs_src_raw);
            free(fs_src_raw);
            if (fs_src)
            {
                GLuint fs = compile(GL_FRAGMENT_SHADER, fs_src);
                if (fs)
                {
                    GLuint new_prog = link_program(
                        compile(GL_VERTEX_SHADER, vs_src), fs);
                    if (new_prog)
                    {
                        if (prog)
                            glDeleteProgram(prog);
                        prog = new_prog;
                        fprintf(stderr, "[INFO] Shader reloaded\n");
                    }
                    glDeleteShader(fs);
                }
                free(fs_src);
            }
        }

        int W, H;
        glfwGetFramebufferSize(window, &W, &H);
        glViewport(0, 0, W, H);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniform2f(glGetUniformLocation(prog, "iResolution"), W, H);
        glUniform1f(glGetUniformLocation(prog, "iTime"),
                    (float)(glfwGetTime() - t0));

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (prog)
        glDeleteProgram(prog);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return 0;
}
