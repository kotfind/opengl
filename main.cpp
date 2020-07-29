#define GL_GLEXT_PROTOTYPES

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

SDL_Window *win;
SDL_GLContext cont;

struct Shader {
    GLuint shader;
    GLenum type;
    GLchar *source;

    Shader(const GLenum type) {
        shader = glCreateShader(type);
        this->type = type;
    }

    operator GLuint() const { return shader; }

    void load(const char* path) {
        FILE *file = NULL;
        file = fopen(path, "r");
        if (file == NULL) {
            cerr << "ERROR::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::SOURCE_FILE_CANNOT_BE_OPENED" << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }

        fseek(file, 0L, SEEK_END);
        long int size = ftell(file) + 1;
        rewind(file);

        source = (GLchar*) malloc(size * sizeof(GLchar));
        for (int i = 0; i < size; ++i) {
            source[i] = fgetc(file);
        }
        source[size - 1] = '\0';

        fclose(file);
    }

    void compile() {
        glShaderSource(shader, 1, (const GLchar**)&source, NULL);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetShaderInfoLog(shader, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
            cerr << "ERROR::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n" << infoLog << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }
    }

    ~Shader() {
        free((void*)source);
        glDeleteShader(shader);
    }
};

struct ShaderProgram {
    GLuint program;

    ShaderProgram(const char *vertexShaderPath, const char *fragmentShaderPath) {
        program = glCreateProgram();

        Shader *vertexShader = new Shader(GL_VERTEX_SHADER);
        vertexShader->load(vertexShaderPath);
        vertexShader->compile();

        Shader *fragmentShader = new Shader(GL_FRAGMENT_SHADER);
        fragmentShader->load(fragmentShaderPath);
        fragmentShader->compile();

        attachShader(vertexShader);
        attachShader(fragmentShader);
        link();

        delete vertexShader;
        delete fragmentShader;
    }

    operator GLuint() const { return program; }

    void attachShader(const Shader *shader) { glAttachShader(program, *shader); }

    void link() {
        glLinkProgram(program);

        int success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[1024];
            glGetProgramInfoLog(program, sizeof(infoLog)/sizeof(infoLog[0]), NULL, infoLog);
            cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << endl;

            SDL_GL_DeleteContext(cont);
            SDL_DestroyWindow(win);
            SDL_Quit();
            exit(1);
        }
    }

    void use() { glUseProgram(program); }

    void set1i(const char *name, const int val) { glUniform1i(glGetUniformLocation(program, name), val); }
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
            800, 800,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    cont = SDL_GL_CreateContext(win);

    // Setting up shaders
    ShaderProgram *program = new ShaderProgram("vertex.glsl", "fragment.glsl");

    // Setting up vertices, VBO & VAO
    float vert[] = {
        // Coordinates        // Colors           // Texture coordinates
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3,
    };

    GLuint VAOs[1], VBOs[1], EBOs[1];
    glGenVertexArrays(1, VAOs);
    glGenBuffers     (1, VBOs);
    glGenBuffers     (1, EBOs);

    glBindVertexArray(VAOs[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Coordinates attrib
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Colors attrib
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Texture coordinates attrib
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // Setting up textures
    Texture *texture1 = new Texture("box.jpg", GL_RGB, GL_RGB);
    Texture *texture2 = new Texture("face.jpg", GL_RGB, GL_RGB);

    program->use();
    program->set1i("texture1", 0);
    program->set1i("texture2", 1);

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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, *texture2);

        program->use();
        glBindVertexArray(VAOs[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        SDL_GL_SwapWindow(win);
    }

    glDeleteVertexArrays(1, VAOs);
    glDeleteBuffers     (1, VBOs);

    SDL_GL_DeleteContext(cont);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
