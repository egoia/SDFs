#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <memory>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "orbit_camera.hpp"
#include "program.hpp"
#include "SI/SDFs.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <cstdio>

#define WIDTH 1280
#define HEIGHT 720

enum class PrimitiveType {
    Sphere,
    Box,
    Plane,
    Cylinder,
    Capsule,
    Cone,
    Torus
};

int on_resize_window_callback(void* userdata, SDL_Event* event){
    if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED){
            glViewport(0, 0, event->window.data1, event->window.data2);
    }
    return 0;
};

std::unique_ptr<NodeSDF> BuildDemoScene()
{
    const Vector up(0.0, 1.0, 0.0);
    const Vector centers[] = {
        Vector(-9.0,  2.0, 0.0), Vector(-6.0,  2.0, 0.0),
        Vector(-3.0,  2.0, 0.0), Vector( 0.0,  2.0, 0.0),
        Vector( 3.0,  2.0, 0.0), Vector( 6.0,  2.0, 0.0),
        Vector( 9.0,  2.0, 0.0), Vector(-9.0, -2.0, 0.0),
        Vector(-6.0, -2.0, 0.0), Vector(-3.0, -2.0, 0.0)
    };

    std::vector<std::unique_ptr<NodeSDF>> objects;
    objects.reserve(10);

    // Ligne du haut : une instance de chaque primitive.
    objects.push_back(std::make_unique<SphereSDF>(centers[0], 0.55f));
    objects.push_back(std::make_unique<BoxSDF>(
        centers[1], Vector(0.48, 0.58, 0.42), up));
    objects.push_back(std::make_unique<CylindreSDF>(
        centers[2] + Vector(0.0, -0.55, 0.0),
        centers[2] + Vector(0.0,  0.55, 0.0), 0.38f));
    objects.push_back(std::make_unique<CapsuleSDF>(
        centers[3] + Vector(-0.4, -0.3, 0.0),
        centers[3] + Vector( 0.4,  0.3, 0.0), 0.27f));
    objects.push_back(std::make_unique<ConeSDF>(
        centers[4] + Vector(0.0, -0.55, 0.0),
        centers[4] + Vector(0.0,  0.55, 0.0), 0.5f));
    objects.push_back(std::make_unique<DonutSDF>(
        centers[5], 0.42f, 0.17f, Vector(0.0, 0.0, 1.0)));

    // Ligne du bas : un exemple de chaque operateur.
    std::unique_ptr<NodeSDF> unionLeft = std::make_unique<SphereSDF>(
        centers[6] + Vector(-0.28, 0.0, 0.0), 0.4f);
    std::unique_ptr<NodeSDF> unionRight = std::make_unique<SphereSDF>(
        centers[6] + Vector(0.28, 0.0, 0.0), 0.4f);
    objects.push_back(std::make_unique<UnionSDF>(unionLeft, unionRight));

    std::unique_ptr<NodeSDF> intersectionLeft =
        std::make_unique<SphereSDF>(centers[7], 0.58f);
    std::unique_ptr<NodeSDF> intersectionRight = std::make_unique<BoxSDF>(
        centers[7], Vector(0.45), up);
    objects.push_back(std::make_unique<IntersectionSDF>(
        intersectionLeft, intersectionRight));

    std::unique_ptr<NodeSDF> subtractionLeft =
        std::make_unique<SphereSDF>(centers[8], 0.58f);
    std::unique_ptr<NodeSDF> subtractionRight = std::make_unique<SphereSDF>(
        centers[8] + Vector(0.55, 0.0, 0.0), 0.48f);
    objects.push_back(std::make_unique<SubstractionSDF>(
        subtractionLeft, subtractionRight));

    std::unique_ptr<NodeSDF> blendLeft = std::make_unique<SphereSDF>(
        centers[9] + Vector(-0.3, 0.0, 0.0), 0.4f);
    std::unique_ptr<NodeSDF> blendRight = std::make_unique<SphereSDF>(
        centers[9] + Vector(0.3, 0.0, 0.0), 0.4f);
    objects.push_back(std::make_unique<BlendSDF>(blendLeft, blendRight, 0.2f));

    std::unique_ptr<NodeSDF> scene = std::move(objects[0]);
    for (std::size_t i = 1; i < objects.size(); ++i) {
        scene = std::make_unique<UnionSDF>(scene, objects[i]);
    }
    return scene;
}

std::unique_ptr<NodeSDF> BuildPerformanceScene(PrimitiveType type)
{
    int objectCount = 1000;
    Vector center(0.0, 0.0, 0.0);
    float radius = 0.1f;
    double halfHeight = 0.1;

    std::vector<std::unique_ptr<NodeSDF>> objects;
    objects.reserve(objectCount);

    for (int i = 0; i < objectCount; ++i) {
        std::unique_ptr<NodeSDF> object;

        switch (type) {
        case PrimitiveType::Sphere:
            object = std::make_unique<SphereSDF>(center, radius);
            break;
        case PrimitiveType::Box:
            object = std::make_unique<BoxSDF>(
                center, Vector(0.1), Vector(0.0, 1.0, 0.0));
            break;
        case PrimitiveType::Plane:
            object = std::make_unique<PlaneSDF>(
                center, Vector(0.0, 1.0, 0.0));
            break;
        case PrimitiveType::Cylinder:
            object = std::make_unique<CylindreSDF>(
                center + Vector(0.0, -halfHeight, 0.0),
                center + Vector(0.0, halfHeight, 0.0), radius);
            break;
        case PrimitiveType::Capsule:
            object = std::make_unique<CapsuleSDF>(
                center + Vector(-halfHeight, 0.0, 0.0),
                center + Vector(halfHeight, 0.0, 0.0), radius);
            break;
        case PrimitiveType::Cone:
            object = std::make_unique<ConeSDF>(
                center + Vector(0.0, -halfHeight, 0.0),
                center + Vector(0.0, halfHeight, 0.0), radius);
            break;
        case PrimitiveType::Torus:
            object = std::make_unique<DonutSDF>(
                center, radius, radius * 0.35f, Vector(0.0, 0.0, 1.0));
            break;
        }

        objects.push_back(std::move(object));
    }

    while (objects.size() > 1) {
        std::vector<std::unique_ptr<NodeSDF>> nextLevel;
        nextLevel.reserve((objects.size() + 1) / 2);

        for (std::size_t i = 0; i < objects.size(); i += 2) {
            if (i + 1 < objects.size()) {
                nextLevel.push_back(
                    std::make_unique<UnionSDF>(objects[i], objects[i + 1]));
            } else {
                nextLevel.push_back(std::move(objects[i]));
            }
        }

        objects = std::move(nextLevel);
    }

    return std::move(objects.front());
}

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

    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glClearDepthf(1.0f);

    //Pas besoin de Zbuffer et ztest
    //Pas besoin de culling

    //transparence (pas nécessaire si discard dans le shader)
    /*glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/


    //Camera
    GLfloat camera_target_dist = 3.0f;
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    OrbitCamera cam(camera_target, camera_target_dist, 45.0f, 0.1f, 1000.0f, WIDTH, HEIGHT,0.3f);

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

    std::unique_ptr<NodeSDF> scene = BuildPerformanceScene(PrimitiveType::Sphere);
    std::unique_ptr<SphereSDF> sphere = std::make_unique<SphereSDF>(Vector(0.0), 1.0f);
    sphere->BenchmarkEvaluation(100, 1);
    
    Program program(
        "assets/shaders/test.vert", "assets/shaders/test.frag",
        {"assets/shaders/SDF_utils.glsl"}, *scene);
    program.use();

    GLint resolutionLocation = glGetUniformLocation(program.ID, "uResolution");
    GLint camPosLocation = glGetUniformLocation(program.ID, "camPos");

    GLint backgroundColorLocation = glGetUniformLocation(program.ID, "backgroundColor");
    glUniform4f(backgroundColorLocation, 0.8f, 0.8f, 0.8f, 1.0f);

    GLint invMVPLocation = glGetUniformLocation(program.ID, "invMVP");




    bool running = true;
    SDL_Event event;
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 start = SDL_GetPerformanceCounter();
    Uint64 frames = 0;

    while (running) {

        //EVENTS
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if(event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_LMASK){
                cam.orbit(event.motion.xrel, event.motion.yrel);
            } else if (event.type == SDL_MOUSEWHEEL) {
                cam.zoom(static_cast<float>(event.wheel.y));
            }
        }

        //OPENGL
        glClear(GL_COLOR_BUFFER_BIT);   // effacer l'image
        glClear(GL_DEPTH_BUFFER_BIT);   // effacer le zbuffer, si nécessaire


        mat4 invMVP = inverse(cam.getProject() * cam.getView());
        glUniformMatrix4fv(invMVPLocation, 1, GL_FALSE, value_ptr(invMVP));

        int drawableWidth;
        int drawableHeight;
        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
        glUniform2f(resolutionLocation, static_cast<float>(drawableWidth), static_cast<float>(drawableHeight));
        
        glUniform3fv(camPosLocation, 1, value_ptr(cam.wrld_position));

        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(window);

        //PERF
        frames++;
        const Uint64 now = SDL_GetPerformanceCounter();

        if (now - start >= frequency) { //CALCUL CHAQUE SECONDE
            double realTimeMs = 1000.0 * (now - start) / frequency; 
            double fps = frames * 1000.0 / realTimeMs;
            double frameDuration = realTimeMs / frames;

            char title[128];
            std::snprintf(title, sizeof(title), "SDF - %.1f FPS | %.2f ms/frame", fps, frameDuration);
            SDL_SetWindowTitle(window, title);

            start = now;
            frames = 0;
        }
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_DelEventWatch(on_resize_window_callback, NULL);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}