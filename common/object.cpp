#include "object.h"

#define SQ(x) (x)*(x)

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  vertices.push_back(a);
  vertices.push_back(b);
  vertices.push_back(c);
  normal = glm::normalize(glm::cross(b - a, c - a));
  normals.push_back(normal);
  normals.push_back(normal);
  normals.push_back(normal);
}

Triangle::Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 na, glm::vec3 nb, glm::vec3 nc) {
  vertices.push_back(a);
  vertices.push_back(b);
  vertices.push_back(c);
  normal = -glm::normalize(glm::cross(b - a, c - a));
  normals.push_back(normal);
  normals.push_back(normal);
  normals.push_back(normal);
}

bool Triangle::contains(glm::vec3 p) const {
  glm::vec3 p0 = vertices[0] - p;
  glm::vec3 p1 = vertices[1] - p;
  glm::vec3 p2 = vertices[2] - p;
  double a_ = glm::length(glm::cross(p1, p2));
  double b_ = glm::length(glm::cross(p2, p0));
  double c_ = glm::length(glm::cross(p0, p1));
  double a = a_ / (a_ + b_ + c_);
  double b = b_ / (a_ + b_ + c_);
  double c = c_ / (a_ + b_ + c_);
  return (a >= 0.0 && a <= 1.0 && b >= 0.0 && b <= 1.0 && c >= 0.0 && c <= 1.0);
}

std::experimental::optional<glm::vec3> Triangle::intersect(Ray r) const {
  double s = glm::dot(normal, vertices[0] - r.origin) / glm::dot(normal, r.direction);
  if (s < EPSILON) return {};
  glm::vec3 p = r.origin + s * r.direction;
  glm::vec3 n = glm::normalize(glm::cross(vertices[1] - vertices[0], p - vertices[0]));
  for (int i = 0; i < vertices.size(); ++i) {
    if (!EQUAL(glm::normalize(glm::cross(vertices[(i+1) % vertices.size()] - vertices[i], p - vertices[i])), n)) return {};
  }
  return p;
}

glm::vec3 Triangle::normalAt(glm::vec3 p) const {
  glm::vec3 p0 = vertices[0] - p;
  glm::vec3 p1 = vertices[1] - p;
  glm::vec3 p2 = vertices[2] - p;
  double a = glm::length(glm::cross(p1, p2));
  double b = glm::length(glm::cross(p2, p0));
  double c = glm::length(glm::cross(p0, p1));
  return (a * normals[0] + b * normals[1] + c * normals[2]) / (a + b + c);
}

Ray Triangle::reflect(Ray ray, double n) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
  if (glm::dot(N, L) < -EPSILON) {
    N = -N;
    n = ray.n;
  }
  glm::vec3 R = glm::dot(2.0 * L, N) * N - L;
  return Ray(q, R, ray.n);
}

Ray Triangle::refract(Ray ray, double n) const {
  glm::vec3 q = intersect(ray).value();
  glm::vec3 L = -ray.direction;
  glm::vec3 N = normalAt(q);
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

Polygon::Polygon(std::vector<glm::vec3> vertices, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
    : Object(ambient, diffuse, specular, gloss, n, reflective, refractive) {
  assert(vertices.size() % 3 == 0);
  for (size_t i = 0; i < vertices.size() / 3; ++i) {
    glm::vec3 a = vertices[i * 3];
    glm::vec3 b = vertices[i * 3 + 1];
    glm::vec3 c = vertices[i * 3 + 2];
    planes.push_back(Triangle(a, b, c));
  }
};

Polygon::Polygon(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, double gloss, double n, bool reflective, bool refractive)
    : Object(ambient, diffuse, specular, gloss, n, reflective, refractive) {
  assert(vertices.size() % 3 == 0);
  assert(vertices.size() == normals.size());
  for (size_t i = 0; i < vertices.size() / 3; ++i) {
    glm::vec3 a = vertices[i * 3];
    glm::vec3 b = vertices[i * 3 + 1];
    glm::vec3 c = vertices[i * 3 + 2];

    glm::vec3 na = normals[i * 3];
    glm::vec3 nb = normals[i * 3 + 1];
    glm::vec3 nc = normals[i * 3 + 2];
    planes.push_back(Triangle(a, b, c, na, nb, nc));
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

glm::vec3 Polygon::normalAt(glm::vec3 q) const {
  for(auto const & p: planes) {
    if (p.contains(q)) {
      return p.normalAt(q);
    }
  }
}

std::experimental::optional<glm::vec3> Polygon::intersect(Ray r) const {
  std::experimental::optional<glm::vec3> result = {};
  for (auto const & p: planes) {
    auto q = p.intersect(r);
    if (q && (!result || glm::distance(r.origin, q.value()) < glm::distance(r.origin, result.value()))) {
      result = q.value();
    }
  }
  return result;
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
