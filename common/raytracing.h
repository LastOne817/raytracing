#ifndef GRAPHICS_RAYTRACING_H
#define GRAPHICS_RAYTRACING_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "object.h"

#include <vector>
#include <png.h>
#include <pthread.h>

#define DEPTH_MAX 10

#define NTHREAD 16

class Light {
public:
    glm::vec3 position;
    double power;
    Light(glm::vec3 position, double power) : position(position), power(power) {};
};

class World {
private:
    std::vector<Object*> objects;
    std::vector<Light> lights;
    std::experimental::optional<Object*> intersect(Ray ray) const;
    glm::vec3 accumulateLightSource(Object* obj, Ray ray) const;
    bool reachable(Light light, glm::vec3 target) const;
    glm::vec3 eye;
public:
    glm::vec3 trace(Ray ray, int depth) const;
    void addObject(Object* obj) { objects.push_back(obj); }
    void addLight(Light& light) { lights.push_back(light); }
    void createImageFromView(glm::vec3 eye, glm::vec3 direction, glm::vec3 up, double view_width, int width, int height);
    glm::vec3 calculateColor(int x, int y, glm::vec3 direction, glm::vec3 right, glm::vec3 up, int width, int height, double view_width, double view_height) const;
};

struct thread_data {
    int id;
    int startIndex;
    int width, height;
    World* world;
    png_byte* image;
    glm::vec3 direction, right, up;
    double view_width, view_height;
};

void* fillImage(void* arg);

#endif //GRAPHICS_RAYTRACING_H
