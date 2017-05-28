#ifndef GRAPHICS_OBJECT_H
#define GRAPHICS_OBJECT_H

#include <vector>
#include <experimental/optional>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define EPSILON 1.0e-2f
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
    Object (glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
        : ambient(ambient), diffuse(diffuse), specular(specular), gloss(gloss), n(n), reflective(reflective), refractive(refractive), reflectWeight(0.1) {};
    virtual std::experimental::optional<glm::vec3> intersect(Ray r) const = 0;
    virtual glm::vec3 normalAt(glm::vec3) const = 0;
    virtual Ray reflect(Ray ray) const = 0;
    virtual Ray refract(Ray ray) const = 0;
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
};

class Polygon : Object {
private:
    glm::vec3 normal;
    glm::vec3 point;
    std::vector<glm::vec3> vertices;
public:
    Polygon(std::vector<glm::vec3> vertices, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive);
    std::experimental::optional<glm::vec3> intersect(Ray r) const;
    glm::vec3 normalAt(glm::vec3) const;
    Ray reflect(Ray ray) const;
    Ray refract(Ray ray) const;
};

#endif //GRAPHICS_OBJECT_H
