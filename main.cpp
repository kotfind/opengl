#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

SDL_Window *win;
SDL_GLContext cont;

struct Shader {
    GLuint shader;
    GLenum type;
    GLchar *source;

    Shader(const GLenum type, const char* path) {
        // Create shader
        this->type = type;
        this->shader = glCreateShader(this->type);
        
        // Load source
        FILE *file = NULL;
        file = fopen(path, "r");
        if (file == NULL) {
            cerr << "ERROR::SHADER::" << (this->type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::SOURCE_FILE_CANNOT_BE_OPENED" << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }

        fseek(file, 0L, SEEK_END);
        long int size = ftell(file) + 1;
        rewind(file);

        this->source = (GLchar*) malloc(size * sizeof(GLchar));
        for (int i = 0; i < size; ++i) {
            this->source[i] = fgetc(file);
        }
        this->source[size - 1] = '\0';

        fclose(file);

        // Compile
        glShaderSource(*this, 1, (const GLchar**)&(this->source), NULL);
        glCompileShader(*this);

        int success;
        glGetShaderiv(*this, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(*this, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
            cerr << "ERROR::SHADER::" << (this->type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n" << infoLog << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }
    }

    operator GLuint() const { return this->shader; }

    ~Shader() {
        free((void*)(this->source));
        glDeleteShader(*this);
    }
};

struct ShaderProgram {
    GLuint program;

    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
        // Create program
        this->program = glCreateProgram();

        // Create shaders
        Shader *vertexShader = new Shader(GL_VERTEX_SHADER, vertexShaderPath);
        Shader *fragmentShader = new Shader(GL_FRAGMENT_SHADER, fragmentShaderPath);

        // Attach Shaders
        glAttachShader(*this, *vertexShader);
        glAttachShader(*this, *fragmentShader);

        // Link
        glLinkProgram(*this);

        int success;
        glGetProgramiv(*this, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(*this, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
            cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }

        delete vertexShader;
        delete fragmentShader;
    }

    operator GLuint() const { return this->program; }

    void use() { glUseProgram(*this); }

    void set1i(const char *name, const GLint val) { glUniform1i(glGetUniformLocation(*this, name), val); }
    void setMatrix4fv(const char *name, const int count, const GLfloat *val) { glUniformMatrix4fv(glGetUniformLocation(*this, name), count, GL_FALSE, val); }
};

struct Texture {
    GLuint texture;

    Texture(const char *path, const GLenum format, const GLenum loadformat) {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0); 
        if (!data) {
            cout << "ERROR::TEXTURE::IMAGE_CANNOT_BE_LOADED" << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);

            stbi_image_free(data);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, loadformat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    operator int() const { return texture; }
};

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800, 600,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    cont = SDL_GL_CreateContext(win);

    // Setting up opengl
    glEnable(GL_DEPTH_TEST);

    // Setting up shaders
    ShaderProgram *program = new ShaderProgram("vertex.glsl", "fragment.glsl");

    // Setting up vertices, VBO & VAO
    const float vert[] = {
        // Coordinates        // Texture coordinates
         0.5f,  0.5f, -0.5f,   1.f, 1.f,
         0.5f, -0.5f, -0.5f,   1.f, 0.f,
        -0.5f, -0.5f, -0.5f,   0.f, 0.f,
        -0.5f,  0.5f, -0.5f,   0.f, 1.f,

         0.5f,  0.5f,  0.5f,   1.f, 1.f,
         0.5f, -0.5f,  0.5f,   1.f, 0.f,
        -0.5f, -0.5f,  0.5f,   0.f, 0.f,
        -0.5f,  0.5f,  0.5f,   0.f, 1.f,

         0.5f,  0.5f, -0.5f,   1.f, 1.f,
         0.5f, -0.5f, -0.5f,   1.f, 0.f,
         0.5f, -0.5f,  0.5f,   0.f, 0.f,
         0.5f,  0.5f,  0.5f,   0.f, 1.f,

        -0.5f, -0.5f, -0.5f,   1.f, 1.f,
        -0.5f,  0.5f, -0.5f,   1.f, 0.f,
        -0.5f,  0.5f,  0.5f,   0.f, 0.f,
        -0.5f, -0.5f,  0.5f,   0.f, 1.f,

         0.5f, -0.5f, -0.5f,   1.f, 1.f,
        -0.5f, -0.5f, -0.5f,   1.f, 0.f,
        -0.5f, -0.5f,  0.5f,   0.f, 0.f,
         0.5f, -0.5f,  0.5f,   0.f, 1.f,

         0.5f,  0.5f, -0.5f,   1.f, 1.f,
        -0.5f,  0.5f, -0.5f,   1.f, 0.f,
        -0.5f,  0.5f,  0.5f,   0.f, 0.f,
         0.5f,  0.5f,  0.5f,   0.f, 1.f,
    };

    const unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3,

        4, 5, 7,
        5, 6, 7,

        8, 9, 11,
        9, 10, 11,

        12, 13, 15,
        13, 14, 15,

        16, 17, 19,
        17, 18, 19,

        20, 21, 23,
        21, 22, 23,
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers     (1, &VBO);
    glGenBuffers     (1, &EBO);

    glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Coordinates attrib
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinates attrib
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for(;;) {
        // Count time
        float time = SDL_GetTicks() / 1000.f;

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program->use();

        // Transformations
        mat4 projection = perspective(float(M_PI) / 3.f, float(W) / float(H), 0.1f, 100.f);
        program->setMatrix4fv("projection", 1, value_ptr(projection));

        mat4 view = lookAt(vec3(0.f, 0.f, -5.f),
                           vec3(0.f),
                           vec3(0.f, 1.f, 0.f));
        program->setMatrix4fv("view", 1, value_ptr(view));

        mat4 model = mat4(1.f);
        model = rotate(model, time * 2.f, vec3(0.5f, 1.f, 0.0f));
        
        program->setMatrix4fv("model", 1, value_ptr(model));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        SDL_GL_SwapWindow(win);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers     (1, &VBO);

    SDL_GL_DeleteContext(cont);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
