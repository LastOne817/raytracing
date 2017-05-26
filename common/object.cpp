#include "object.h"

Polygon::Polygon(std::vector<glm::vec3> vertices, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
    : vertices(vertices), Object(ambient, diffuse, specular, gloss, n, reflective, refractive) {
  glm::vec3 sum = glm::vec3(0);
  for (auto const& vertex: vertices) {
    sum += vertex;
  }
  point = sum / vertices.size();
  glm::vec3 e1 = vertices[1] - vertices[0];
  glm::vec3 e2 = vertices[2] - vertices[1];
  normal = glm::normalize(glm::cross(e1, e2));
  for (size_t i = 1; i < vertices.size(); ++i) {
    glm::vec3 ea = vertices[(i + 1) % vertices.size()] - vertices[i];
    glm::vec3 eb = vertices[(i + 2) % vertices.size()] - vertices[(i + 1) % vertices.size()];
    glm::vec3 n = glm::normalize(glm::cross(ea, eb));
    assert(EQUAL(n, normal));
  }
};

std::experimental::optional<glm::vec3> Sphere::intersect(Ray r) const {
  glm::vec3 dp = center - r.origin;
  double udp = glm::dot(r.direction, dp);
  double det = udp * udp - glm::dot(dp, dp) + radius * radius;
  if (det >= 0.0) {
    double s = udp - glm::sqrt(det);
    return r.origin + r.direction * s;
  }
  else return {};
}

glm::vec3 Sphere::normalAt(glm::vec3 p) const {
  return glm::normalize(p - center);
}

glm::vec3 Polygon::normalAt(glm::vec3) const {
  return normal;
}

std::experimental::optional<glm::vec3> Polygon::intersect(Ray r) const {
  double s = glm::dot(normal, point - r.origin) / glm::dot(normal, r.direction);
  if (s < 0.0) return {};
  glm::vec3 p = r.origin + s * r.direction;
  glm::vec3 n = glm::normalize(glm::cross(vertices[1] - vertices[0], p - vertices[0]));
  for (int i = 0; i < vertices.size(); ++i) {
    if (!EQUAL(glm::normalize(glm::cross(vertices[(i+1) % vertices.size()] - vertices[i], p - vertices[i])), n)) return {};
  }
  return p;
}

Ray Sphere::reflect(Ray ray) const {
  return ray;
}

Ray Sphere::refract(Ray ray) const {
  return ray;
}

Ray Polygon::reflect(Ray ray) const {
  return ray;
}

Ray Polygon::refract(Ray ray) const {
  return ray;
}
