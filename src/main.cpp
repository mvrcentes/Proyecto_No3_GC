#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include "skybox.h"
#include "light.h"
#include "color.h"
#include "object.h"
#include "sphere.h"
#include "cube.h"
#include "camera.h"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400
#define FOV glm::radians(90.0f)  // Field of view is 90 degrees
#define ASPECT_RATIO (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT
#define BIAS 0.01f
#define MAX_RECURSION_DEPTH 2

SDL_Renderer* renderer = nullptr;
Light light(glm::vec3(0.0f, 14.0f, -60.0f), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(-2.0f, 14.0f, -30.0f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
Skybox skybox("./textures/skybox.jpg");

float castShadow(const glm::vec3& shadowOrig, const glm::vec3& lightDir, 
                 const std::vector<Object*>& objects, Object* hitObject) {
    for (auto& obj : objects) {
        if (obj != hitObject) {
            // Add a small bias to the origin to prevent shadow acne
            glm::vec3 biasedOrigin = shadowOrig + BIAS * lightDir;

            Intersect shadowIntersect = obj->rayIntersect(biasedOrigin, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.distance > 0) {
                // Calculate the shadow intensity based on the distance to the intersecting object
                // The intensity decreases as the object is closer to the shadow origin
                float lightDistance = glm::length(light.position - shadowOrig);
                float shadowFactor = shadowIntersect.distance / lightDistance;
                shadowFactor = glm::clamp(shadowFactor, 0.0f, 1.0f); // Clamp between 0 and 1

                // Calculate final shadow intensity
                const float shadowIntensity = 1.0f - shadowFactor;
                return shadowIntensity;
            }
        }
    }

    return 1.0f; // No shadow
}

Color computeShading(const glm::vec3& orig, const glm::vec3& dir, const Intersect& intersect, 
                     Object* hitObject, const std::vector<Object*>& objects, const short recursion);

Color castRay(const glm::vec3& orig, const glm::vec3& dir, 
              const std::vector<Object*>& objects, const short recursion = 0) {
    Intersect closestIntersect;
    Object* hitObject = nullptr;
    float closestDistance = std::numeric_limits<float>::infinity();

    // Find the closest intersecting object
    for (auto& obj : objects) {
        Intersect intersect = obj->rayIntersect(orig, dir);
        if (intersect.isIntersecting && intersect.distance < closestDistance) {
            closestDistance = intersect.distance;
            closestIntersect = intersect;
            hitObject = obj;
        }
    }

    // Return sky color if no intersection or max recursion depth reached
    if (!closestIntersect.isIntersecting || recursion >= MAX_RECURSION_DEPTH) {
        return skybox.getColor(dir);
    }

    // Compute lighting and shading
    return computeShading(orig, dir, closestIntersect, hitObject, objects, recursion);
}

Color computeShading(const glm::vec3& orig, const glm::vec3& dir, const Intersect& intersect, 
                     Object* hitObject, const std::vector<Object*>& objects, const short recursion) {
    const Material& mat = hitObject->getMaterial();
    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(orig - intersect.point);
    float shadowIntensity = castShadow(intersect.point + BIAS * intersect.normal, lightDir, objects, hitObject);
    float intensity = shadowIntensity * light.intensity;

    // Calculate diffuse and specular components
    float diffIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);
    float specIntensity = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), mat.specularCoefficient);

    Color diffuse = diffIntensity * mat.albedo * mat.diffuse;
    Color specular = specIntensity * mat.specularAlbedo * light.color;

    // Compute reflected and refracted components, if applicable
    Color reflected, refracted;
    if (mat.reflectivity > 0) {
        reflected = mat.reflectivity * castRay(intersect.point + BIAS * intersect.normal, reflectDir, objects, recursion + 1);
    }
    if (mat.transparency > 0) {
        glm::vec3 refractDir = glm::refract(dir, intersect.normal, mat.refractionIndex);
        refracted = mat.transparency * castRay(intersect.point - BIAS * intersect.normal, refractDir, objects, recursion + 1);
    }

    return (1 - mat.reflectivity - mat.transparency) * (diffuse + specular) + reflected + refracted;
}

void pixel(glm::vec2 position, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

void render(std::vector<Object*>& objects) {
    // Camera orientation vectors
    glm::vec3 dir = glm::normalize(camera.target - camera.position);
    glm::vec3 right = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::cross(right, dir);

    // Pre-compute scaling factors for ray direction calculation
    float widthInv = 1.0f / SCREEN_WIDTH;
    float heightInv = 1.0f / SCREEN_HEIGHT;
    float aspectRatio = ASPECT_RATIO;

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            // Convert pixel position to normalized device coordinates (NDC)
            float ndcX = (2.0f * x + 1) * widthInv - 1.0f;
            float ndcY = 1.0f - (2.0f * y + 1) * heightInv;

            // Adjust for aspect ratio and compute ray direction
            glm::vec3 rayDir = glm::normalize(dir + right * ndcX * aspectRatio + up * ndcY);

            // Cast the ray and draw the pixel
            pixel(glm::vec2(x, y), castRay(camera.position, rayDir, objects));
        }
    }
}


int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Proyecto 3: Raytracing",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event;
    float rotateAngle = 0.0f;

    unsigned int lastTime = SDL_GetTicks();
    unsigned int currentTime;
    float dT;

    // Stone for the mountain
    Material stone(
        Color(128, 128, 128),
        0.6f, 0.2f, 30.0f, 0.0f
    );

    // Water for the waterfall and river
    // Material waterBlock(
    //     Color(28, 107, 160),
    //     0.8f, 0.6f, 25.0f,
    //     0.1f, 0.2f, 1.33f
    // );

    Material waterBlock(
        Color(28, 107, 160),  // Color azul agua
        0.5f,                 // Albedo moderado
        0.8f,                 // Alto specular albedo para aumentar el brillo
        50.0f,                // Specular coeficiente para resaltar el brillo
        0.5f,                 // Reflectividad aumentada para simular mejor la superficie reflectante del agua
        0.8f,                 // Transparencia incrementada para una refracción más notoria
        1.33f                 // Índice de refracción del agua
    );

    // Leaves for trees
    Material leavesBlock(
        Color(0, 128, 0),
        0.8f, 0.2f, 10.0f, 0.0f,
        0.2f, 1.0f
    );

    // Wood for tree trunks
    Material woodBlock(
        Color(83, 53, 10),
        0.8f, 0.1f, 10.0f, 0.0f
    );

    // Sand for riverbanks
    Material sandBlock(
        Color(194, 178, 128),
        0.8f, 0.1f, 10.0f, 0.0f
    );

    Material glass(
        Color(255, 255, 255),
        0.1f,
        1.0f,
        125.0f,
        0.0f,
        0.9f,
        0.1f
    );

    std::vector<Object*> objects;

    // Define the size of each cube (Minecraft style)
    float cubeSize = 2.0f;

    // Larger Mountain Landscape
    int landscapeSize = 10;  // Increase landscape size
    for (int x = -landscapeSize; x <= landscapeSize; x += 2) {
        for (int z = -landscapeSize; z <= landscapeSize; z += 2) {
            float height = std::max(0.0f, 8.0f - glm::length(glm::vec2(x, z)));  // Adjusted hill shape
            for (int y = 0; y < height; y += 2) {
                objects.push_back(new Cube(
                    glm::vec3(x, y, z),
                    glm::vec3(x + cubeSize, y + cubeSize, z + cubeSize),
                    stone
                ));
            }
        }
    }

    // Base Layer (Dirt or Sand)
    for (int x = -landscapeSize; x <= landscapeSize; x += 2) {
        for (int z = -landscapeSize; z <= landscapeSize; z += 2) {
            objects.push_back(new Cube(
                glm::vec3(x, -2.0f, z),
                glm::vec3(x + cubeSize, 0.0f, z + cubeSize),
                sandBlock  // or sandBlock
            ));
        }
    }

    // Asumimos que la montaña tiene una altura máxima de 8 y empieza a disminuir desde ahí
    int alturaMaxima = 7;
    int xCascada = 0;  // Coordenada X donde comienza la cascada
    int zCascada = -4; // Coordenada Z donde comienza la cascada

    // Construir la cascada
    for (int y = alturaMaxima; y >= 0; y -= 2) {
        // Colocar un cubo de agua en cada paso hacia abajo
        objects.push_back(new Cube(
            glm::vec3(xCascada, y, zCascada),
            glm::vec3(xCascada + cubeSize, y + cubeSize, zCascada + cubeSize),
            waterBlock
        ));

        // Ajustar la posición Z para el siguiente cubo, si es necesario
        zCascada -= 2;
    }

    int yBaseCascada = -2; // La altura en la que termina la cascada
    for (int y = yBaseCascada+2; y >= yBaseCascada - 10; y -= 2) {
        objects.push_back(new Cube(
            glm::vec3(xCascada, y, zCascada),
            glm::vec3(xCascada + cubeSize, y + cubeSize, zCascada + cubeSize),
            waterBlock
        ));
    }

    // Trees spread out across the landscape
    std::vector<glm::vec2> treePositions = {{-8, 8}, {10, -10}, {-10, 10}, {8, -8}, {-6, -6}};
    for (auto& pos : treePositions) {
        // Tree trunk
        for (int y = 0; y <= 4; y += 2) {
            objects.push_back(new Cube(
                glm::vec3(pos.x, y, pos.y),
                glm::vec3(pos.x + cubeSize, y + cubeSize, pos.y + cubeSize),
                woodBlock
            ));
        }
        // Tree leaves
        for (int x = pos.x - 2; x <= pos.x + 2; x += 2) {
            for (int z = pos.y - 2; z <= pos.y + 2; z += 2) {
                if (x != pos.x || z != pos.y) {  // Avoid the center top of the trunk
                    objects.push_back(new Cube(
                        glm::vec3(x, 6.0f, z),
                        glm::vec3(x + cubeSize, 8.0f, z + cubeSize),
                        leavesBlock
                    ));
                }
            }
        }
    }

    objects.push_back(
        new Sphere(
            glm::vec3(-12.0f, 8.0f, -10.0f),
            2.0f,
            glass
        ));


    int frameCount = 0;
    float elapsedTime = 0.0f;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_q:
                            isRunning = false;
                            break;
                        case SDLK_UP:
                            // Move closer to the target
                            camera.move(-1.0f);  // You may need to adjust the value as per your needs
                            break;
                        case SDLK_DOWN:
                            // Move away from the target
                            camera.move(1.0f);  // You may need to adjust the value as per your needs
                            break;
                        case SDLK_a:
                            // Rotate up
                            camera.rotate(-1.0f, 0.0f);  // You may need to adjust the value as per your needs
                            break;
                        case SDLK_d:
                            // Rotate down
                            camera.rotate(1.0f, 0.0f);  // You may need to adjust the value as per your needs
                            break;
                        case SDLK_w:
                            // Rotate left
                            camera.rotate(0.0f, -1.0f);  // You may need to adjust the value as per your needs
                            break;
                        case SDLK_s:
                            // Rotate right
                            camera.rotate(0.0f, 1.0f);  // You may need to adjust the value as per your needs
                            break;
                        default:
                            break;
                    }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(objects);

        SDL_RenderPresent(renderer);

        // Calculate the deltaTime
        currentTime = SDL_GetTicks();
        dT = (currentTime - lastTime) / 1000.0f;  // Time since last frame in seconds
        lastTime = currentTime;

        frameCount++;
        elapsedTime += dT;
        if (elapsedTime >= 1.0f) {
            float fps = static_cast<float>(frameCount) / elapsedTime;
            std::cout << "FPS: " << fps << std::endl;

            frameCount = 0;
            elapsedTime = 0.0f;
        }
    }

    for (Object* object : objects) {
        delete object;
    }
    objects.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
