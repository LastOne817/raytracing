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
  if (det > EPSILON) {
    double s = udp - glm::sqrt(det);
    if (s < EPSILON) return {};
    return r.origin + r.direction * s;
  }
  else return {};
}

glm::vec3 Sphere::normalAt(glm::vec3 p) const {
  if (bumpmap) {
    glm::vec3 up = glm::vec3(0,1,0);
    glm::vec3 z = glm::normalize(p - center);
    if (EQUAL(up, z)) return glm::normalize(p - center);
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    glm::vec3 y = glm::cross(z, x);
    double u = this->u(p);
    double v = this->v(p);
    glm::vec3 c = bumpmap->getTexture(u, v);
    glm::vec3 coef = (2.0 * c) - glm::vec3(1.0);
    return glm::normalize(x * coef.x + y * coef.y + z * coef.z);
  }
  else return glm::normalize(p - center);
}

glm::vec3 Polygon::normalAt(glm::vec3) const {
  return normal;
}

std::experimental::optional<glm::vec3> Polygon::intersect(Ray r) const {
  double s = glm::dot(normal, point - r.origin) / glm::dot(normal, r.direction);
  if (s < EPSILON) return {};
  glm::vec3 p = r.origin + s * r.direction;
  glm::vec3 n = glm::normalize(glm::cross(vertices[1] - vertices[0], p - vertices[0]));
  for (int i = 0; i < vertices.size(); ++i) {
    if (!EQUAL(glm::normalize(glm::cross(vertices[(i+1) % vertices.size()] - vertices[i], p - vertices[i])), n)) return {};
  }
  return p;
}

Ray Sphere::reflect(Ray ray) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
  double n = this->n;
  if (glm::dot(N, L) < -EPSILON) {
    N = -N;
    n = ray.n;
  }
  glm::vec3 R = glm::dot(2.0 * L, N) * N - L;
  return Ray(q, R, ray.n);
}

Ray Sphere::refract(Ray ray) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
  double n = this->n;
  if (glm::dot(N, L) < -EPSILON) {
    N = -N;
    n = ray.n;
  }
  double cos_i = glm::dot(L, N);
  double cos_r = sqrt(1.0 - (ray.n / n) * (ray.n / n) * (1.0 - cos_i * cos_i));
  glm::vec3 T = (ray.n / n * cos_i - cos_r) * N - ray.n / n * L;
  assert(EQUAL(T, glm::normalize(T)));
  return Ray(q, T, n);
}

Ray Polygon::reflect(Ray ray) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
  double n = this->n;
  if (glm::dot(N, L) < -EPSILON) {
    N = -N;
    n = ray.n;
  }
  glm::vec3 R = glm::dot(2.0 * L, N) * N - L;
  return Ray(q, R, ray.n);
}

Ray Polygon::refract(Ray ray) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
  double n = this->n;
  if (glm::dot(N, L) < -EPSILON) {
    N = -N;
    n = ray.n;
  }
  double cos_i = glm::dot(L, N);
  double cos_r = sqrt(1.0 - (ray.n / n) * (ray.n / n) * (1.0 - cos_i * cos_i));
  glm::vec3 T = (ray.n / n * cos_i - cos_r) * N - ray.n / n * L;
  assert(EQUAL(T, glm::normalize(T)));
  return Ray(q, T, n);
}

double Sphere::u(glm::vec3 point) const {
  glm::vec3 p = (point - center) / radius;
  return p.x;
}

double Sphere::v(glm::vec3 point) const {
  glm::vec3 p = (point - center) / radius;
  return p.y;
}

double Polygon::u(glm::vec3) const {
  return 0.0;
}

double Polygon::v(glm::vec3) const {
  return 0.0;
}
