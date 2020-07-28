#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

using namespace std;

SDL_Window *win;

const char* frag_shader_source =\
"#version 330 core\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

const char* vert_shader_source =\
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_GLContext cont = SDL_GL_CreateContext(win);

    // Compile shader program
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
    glCompileShader(vert_shader);

    int success;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(vert_shader, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
    glCompileShader(frag_shader);

    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(frag_shader, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
        cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);  

    // Setting up vertices, VBO & VAO
    float vert[] = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for(;;) {
        // Count resolution & set viewport
        int W, H;
        SDL_GetWindowSize(win, &W, &H);
        glViewport(0, 0, W, H);

        // Event loop
        bool quit = 0;
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        if (quit) break;

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(win);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    SDL_GL_DeleteContext(cont);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
