// Visualisation : objet erode de maniere incrementale (a gauche) et par paquets (a droite)

#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "orbit_camera.hpp"
#include "program.hpp"
#include "SI/SDFs.hpp"
#include "SI/erosion.hpp"

#define WIDTH 1280
#define HEIGHT 720

int on_resize_window_callback(void* userdata, SDL_Event* event){
    if(event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED){
            glViewport(0, 0, event->window.data1, event->window.data2);
    }
    return 0;
};

// Polygonise la scene et ajoute ses sommets position/normale a vertices,
// decales de offset pour pouvoir afficher plusieurs objets cote a cote.
void AppendMesh(const NodeSDF& scene, const Vector& offset, std::vector<GLfloat>& vertices)
{
    Mesh mesh;
    // Resolution 100 : cellule ~0.012, assez fine pour des crateres de rayon 0.05
    scene.Polygonize(100, mesh, Box(Vector(-0.7), Vector(0.7)));

    for (int triangle = 0; triangle < mesh.Triangles(); ++triangle) {
        for (int corner = 0; corner < 3; ++corner) {
            const Vector position = mesh.Vertex(triangle, corner) + offset;
            const Vector normal = mesh.Normal(mesh.NormalIndex(triangle, corner));
            for (int component = 0; component < 3; ++component) {
                vertices.push_back(static_cast<GLfloat>(position[component]));
            }
            for (int component = 0; component < 3; ++component) {
                vertices.push_back(static_cast<GLfloat>(normal[component]));
            }
        }
    }
}

int main(int argc, char* argv[]) {
    //init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur SDL_Init: " << SDL_GetError() << '\n';
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //Create window
    SDL_Window* window = SDL_CreateWindow(
        "Erosion : incrementale (gauche) / par paquets (droite)",
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

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);

    glViewport(0, 0, WIDTH, HEIGHT);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);

    //Camera
    GLfloat camera_target_dist = 4.0f;
    glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
    OrbitCamera cam(camera_target, camera_target_dist, 45.0f, 0.1f, 1000.0f, WIDTH, HEIGHT,0.3f);

    // Memes parametres que le benchmark : boite de demi-cotes 0.5, meme seed pour les deux methodes
    const Vector sceneCenter(0.0);
    const double sceneRadius = 1.5;
    const double impactRadius = 0.05;
    const unsigned seed = 42;
    const int nbImpacts = 500;
    const int batchSize = 50;

    std::unique_ptr<NodeSDF> incremental = ErodeIncremental(
        std::make_unique<BoxSDF>(Vector(0.0), Vector(0.5), Vector(0.0, 1.0, 0.0)),
        nbImpacts, impactRadius, sceneCenter, sceneRadius, seed);
    std::unique_ptr<NodeSDF> batch = ErodeBatch(
        std::make_unique<BoxSDF>(Vector(0.0), Vector(0.5), Vector(0.0, 1.0, 0.0)),
        nbImpacts, batchSize, impactRadius, sceneCenter, sceneRadius, seed);

    std::cout << "Polygonisation (peut prendre quelques secondes)...\n";
    std::vector<GLfloat> meshVertices;
    AppendMesh(*incremental, Vector(-0.8, 0.0, 0.0), meshVertices);
    AppendMesh(*batch, Vector(0.8, 0.0, 0.0), meshVertices);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(GLfloat), meshVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    Program p("assets/shaders/mesh.vert", "assets/shaders/mesh.frag", {});
    p.use();
    const GLint viewLocation = glGetUniformLocation(p.ID, "uView");
    const GLint projectionLocation = glGetUniformLocation(p.ID, "uProjection");

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEMOTION && (event.motion.state & SDL_BUTTON_LMASK)) {
                cam.orbit(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
            } else if (event.type == SDL_MOUSEWHEEL) {
                cam.zoom(static_cast<float>(event.wheel.y));
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        int drawableWidth;
        int drawableHeight;
        SDL_GL_GetDrawableSize(window, &drawableWidth, &drawableHeight);
        glViewport(0, 0, drawableWidth, drawableHeight);

        const glm::mat4 view = cam.getView();
        const glm::mat4 projection = glm::perspective(
            cam.fov,
            static_cast<float>(drawableWidth) / static_cast<float>(drawableHeight),
            cam.near,
            cam.far
        );
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(meshVertices.size() / 6));

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
