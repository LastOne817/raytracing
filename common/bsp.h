#ifndef GRAPHICS_BSP_H
#define GRAPHICS_BSP_H

#include <vector>
#include <cstdio>
using namespace std;

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <memory>

#define EPSILON 1.0e-5f
#define EQUAL(x,y) (glm::all(glm::lessThan(glm::abs((x) - (y)), glm::vec3(EPSILON))))

class Polygon {
    // Simple, convex polygon
public:
    vector<glm::vec3> points;
    vector<glm::vec3> normals;
    glm::vec3 plane_normal;
    glm::vec3 Kd_value, Ka_value, Ks_value;
    glm::vec4 color;
    float n_value;

    Polygon(vector<glm::vec3>&, glm::vec3, glm::vec3, glm::vec3, float, glm::vec4);
    Polygon(vector<glm::vec3>&,vector<glm::vec3>&, glm::vec3, glm::vec3, glm::vec3, float, glm::vec4);
    vector<Polygon> slice(glm::vec3 & point, glm::vec3 & normal);
    vector<Polygon> slice(Polygon& plane);
    bool isFront(Polygon& other) const;
    bool isBehind(Polygon& other) const;
    bool isOnSamePlane(Polygon& other) const;
    void apply(glm::mat4);
};

class BSPNode {
    using node_ptr = std::unique_ptr<BSPNode>;
    node_ptr front, behind;
    std::vector<Polygon> polygons;
    glm::vec3 normal;
    glm::vec3 point;

public:
    BSPNode(std::vector<Polygon>);
    void print(int indent, int index);
    void apply(glm::mat4);
    bool isFront(glm::vec3&) const;
    bool isBehind(glm::vec3&) const;
    std::vector<Polygon> getPolygons();
};

class BSPTree {
    using node_ptr = std::unique_ptr<BSPNode>;
    node_ptr root;
public:
    BSPTree(std::vector<Polygon> polygons);
    BSPTree(std::vector<glm::vec3> & vertices, glm::vec3, glm::vec3, glm::vec3, float, glm::vec4);
    void print();
    void apply(glm::mat4);
    std::vector<Polygon> getPolygons();
};

#endif //GRAPHICS_BSP_H
