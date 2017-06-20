#ifndef GRAPHICS_OBJECT_H
#define GRAPHICS_OBJECT_H

#include <vector>
#include <experimental/optional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "texture.h"

#define EPSILON 1.0e-3f
#define EQUAL(x,y) (glm::all(glm::lessThan(glm::abs((x) - (y)), glm::vec3(EPSILON))))

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;
    double n;
    Ray(glm::vec3 origin, glm::vec3 direction, double n) : origin(origin), direction(glm::normalize(direction)), n(n) {};
};

class Object {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    double n;
    double gloss;
    bool reflective;
    bool refractive;
    double reflectWeight;
    Texture* texture;
    Texture* bumpmap;

    Object (glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
        : ambient(ambient), diffuse(diffuse), specular(specular), gloss(gloss), n(n),
          reflective(reflective), refractive(refractive), reflectWeight(0.1), texture(nullptr), bumpmap(nullptr) {};
    virtual std::experimental::optional<glm::vec3> intersect(Ray r) const = 0;
    virtual glm::vec3 normalAt(glm::vec3) const = 0;
    virtual Ray reflect(Ray ray) const = 0;
    virtual Ray refract(Ray ray) const = 0;
    virtual double u(glm::vec3) const = 0;
    virtual double v(glm::vec3) const = 0;
};

class Sphere : Object {
private:
    glm::vec3 center;
    double radius;
public:
    Sphere(glm::vec3 center, double radius, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
        : center(center), radius(radius), Object(ambient, diffuse, specular, gloss, n, reflective, refractive) {};
    std::experimental::optional<glm::vec3> intersect(Ray r) const;
    glm::vec3 normalAt(glm::vec3) const;
    Ray reflect(Ray ray) const;
    Ray refract(Ray ray) const;
    double u(glm::vec3) const;
    double v(glm::vec3) const;
};

class Triangle {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    glm::vec3 normal;
public:
    Triangle(glm::vec3, glm::vec3, glm::vec3);
    Triangle(glm::vec3, glm::vec3, glm::vec3, glm::vec3, glm::vec3, glm::vec3);
    std::experimental::optional<glm::vec3> intersect(Ray r) const;
    bool contains(glm::vec3 p) const;
    glm::vec3 normalAt(glm::vec3) const;
    Ray reflect(Ray ray, double n) const;
    Ray refract(Ray ray, double n) const;
};

class Polygon : Object {
private:
    std::vector<Triangle> planes;
public:
    Polygon(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive);
    Polygon(std::vector<glm::vec3> vertices, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive);
    std::experimental::optional<glm::vec3> intersect(Ray r) const;
    glm::vec3 normalAt(glm::vec3) const;
    Ray reflect(Ray ray) const;
    Ray refract(Ray ray) const;
    double u(glm::vec3) const;
    double v(glm::vec3) const;
};

#endif //GRAPHICS_OBJECT_H
