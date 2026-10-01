#pragma once
#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "orbit_camera.hpp"
#include "program.hpp"
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 1280
#define HEIGHT 720

int on_resize_window_callback(void* userdata, SDL_Event* event){
    if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED){
            glViewport(0, 0, event->window.data1, event->window.data2);
    }
    return 0;
};

int main(int argc, char* argv[]) {
    //init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur SDL_Init: " << SDL_GetError() << '\n';
        return -1;
    }

    // Profil OpenGL 4.6 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //Create window
    SDL_Window* window = SDL_CreateWindow(
        "OpenGL Window (Ubuntu / Make)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "Erreur SDL_CreateWindow: " << SDL_GetError() << '\n';
        SDL_Quit();
        return -1;
    }
    SDL_AddEventWatch(on_resize_window_callback, NULL);

    //Context OpenGL
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Erreur SDL_GL_CreateContext: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    //Load glad
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Erreur initialisation Glad\n";
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // configure pipeline
    // choisir dans quelle image dessiner, celle qui n'est pas affichée, GL_BACK
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);

    glViewport(0, 0, WIDTH, HEIGHT);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClearDepthf(1.0f);

    //Pas besoin de Zbuffer et ztest
    //Pas besoin de culling

    //transparence (pas nécessaire si discard dans le shader)
    /*glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/


    //Camera
    GLfloat camera_target_dist = 3.0f;
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    OrbitCamera cam(camera_target, camera_target_dist, 45.0f, 0.1f, 1000.0f, WIDTH, HEIGHT);

    //VAO
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //TODO use a box instead tp improve perforcmances
    const GLfloat fullscreenTriangle[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenTriangle), fullscreenTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    Program program("assets/shaders/test.vert", "assets/shaders/test.frag");
    program.use();

    GLint resolutionLocation = glGetUniformLocation(program.ID, "uResolution");
    GLint camPosLocation = glGetUniformLocation(program.ID, "camPos");

    GLint backgroundColorLocation = glGetUniformLocation(program.ID, "backgroundColor");
    glUniform4f(backgroundColorLocation, 0.168627451f, 0.168627451f, 0.439215686f, 1.0f);

    GLint invMVPLocation = glGetUniformLocation(program.ID, "invMVP");
    mat4 invMVP = inverse(cam.getProject() * cam.getView());
    glUniformMatrix4fv(invMVPLocation, 1, GL_FALSE, value_ptr(invMVP));




    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                //TODO call camera rotation
                running = false;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);   // effacer l'image
        glClear(GL_DEPTH_BUFFER_BIT);   // effacer le zbuffer, si nécessaire

        int drawableWidth;
        int drawableHeight;
        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
        glUniform2f(resolutionLocation, static_cast<float>(drawableWidth), static_cast<float>(drawableHeight));
        glUniform3fv(camPosLocation, 1, value_ptr(cam.wrld_position));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_DelEventWatch(on_resize_window_callback, NULL);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}