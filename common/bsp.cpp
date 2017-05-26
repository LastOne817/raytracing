#include "bsp.h"

Polygon::Polygon(vector<glm::vec3> & point, glm::vec3 Kd, glm::vec3 Ka, glm::vec3 Ks, float n, glm::vec4 color)
    : Kd_value(Kd), Ka_value(Ka), Ks_value(Ks), n_value(n), color(color) {
  if (point.size() < 3) throw -1;
  this->points.clear();
  size_t size = point.size();

  for (int i = 0; i < size; ++i) {
    if (EQUAL(point[i], point[(i+1) % size])) {
      point.erase(point.begin() + i);
      i--;
      size--;
    }
  }
  if (point.size() < 3) throw -1;
  glm::vec3 e1 = point[1] - point[0];
  glm::vec3 e2 = point[2] - point[1];
  plane_normal = glm::normalize(glm::cross(e1, e2));

  for (int i = 1; i < point.size(); ++i) {
    glm::vec3 ea = point[(i + 1) % point.size()] - point[i];
    glm::vec3 eb = point[(i + 2) % point.size()] - point[(i + 1) % point.size()];
    glm::vec3 n = glm::normalize(glm::cross(ea, eb));
    // Check all points are on same plane
    assert(EQUAL(n, plane_normal));
  }
  for(auto const& p: point) {
    this->points.push_back(p);
  }
}

Polygon::Polygon(vector<glm::vec3> & point, vector<glm::vec3> & normals, glm::vec3 Kd, glm::vec3 Ka, glm::vec3 Ks, float n, glm::vec4 color)
    : Kd_value(Kd), Ka_value(Ka), Ks_value(Ks), n_value(n), color(color) {
  if (point.size() < 3) throw -1;
  this->points.clear();
  size_t size = point.size();
  printf("%d\n", point.size());
  for(auto const& p: point) {
    printf("%s\n", glm::to_string(p).c_str());
  }
  printf("\n");

  for (int i = 0; i < size; ++i) {
    if (EQUAL(point[i], point[(i+1) % size])) {
      point.erase(point.begin() + i);
      i--;
      size--;
    }
  }
  if (point.size() < 3) throw -1;
  glm::vec3 e1 = point[1] - point[0];
  glm::vec3 e2 = point[2] - point[1];
  plane_normal = glm::normalize(glm::cross(e1, e2));

  for (int i = 1; i < point.size(); ++i) {
    glm::vec3 ea = point[(i + 1) % point.size()] - point[i];
    glm::vec3 eb = point[(i + 2) % point.size()] - point[(i + 1) % point.size()];
    glm::vec3 n = glm::normalize(glm::cross(ea, eb));
    // Check all points are on same plane
    assert(EQUAL(n, plane_normal));
  }
  glm::vec3 avg = glm::vec3(0.0f);
  for(auto const& n: normals) {
    avg += n;
  }
  avg /= normals.size();

  if (glm::dot(plane_normal, avg) > 0.0f) {
    for (auto const &p: point) {
      this->points.push_back(p);
    }
  }
  else {
    plane_normal *= -1.0f;
    for (size_t i = point.size() - 1; i >= 0; --i) {
      this->points.push_back(point[i]);
    }
  }
  plane_normal *= -1.0f;
}
/**
 * Slice polygon by plane with given point and normal vector
 * @param point
 * @param normal
 * @return
 */
vector<Polygon> Polygon::slice(glm::vec3 & p, glm::vec3 & n) {
  vector<Polygon> result;
  vector<glm::vec3> polygon[2];
  int pidx = 0;
  for (int i = 0; i < points.size(); ++i) {
    auto p1 = points[i];
    auto p2 = points[(i+1) % points.size()];

    polygon[pidx % 2].push_back(p1);
    if (glm::dot(p1 - p, n) * glm::dot(p2 - p, n) < 0.0f) {
      float s = glm::dot(n, p - p1) / glm::dot(n, p2 - p1);
      glm::vec3 pi = p1 + s * (p2 - p1);
      polygon[pidx % 2].push_back(pi);
      pidx++;
      polygon[pidx % 2].push_back(pi);
    }
  }
  try {
    result.push_back(Polygon(polygon[0], Kd_value, Ka_value, Ks_value, n_value, color));
  }
  catch (int _) { }
  if (pidx != 0) {
    try {
      result.push_back(Polygon(polygon[1], Kd_value, Ka_value, Ks_value, n_value, color));
    }
    catch (int _) { }
  }
  return result;
}

vector<Polygon> Polygon::slice(Polygon &plane) {
  return slice(plane.points[0], plane.plane_normal);
}

bool Polygon::isFront(Polygon &other) const {
  // Is this in front of (on normal side) other
  for (glm::vec3 p: points) {
    float direction = glm::dot(p - other.points[0], other.plane_normal);
    if (direction < -EPSILON) return false;
  }
  return true;
}

bool Polygon::isBehind(Polygon &other) const {
  // Is this behind (on opposite of normal side) other
  for (glm::vec3 p: points) {
    float direction = glm::dot(p - other.points[0], other.plane_normal);
    if (direction > EPSILON) return false;
  }
  return true;
}

bool Polygon::isOnSamePlane(Polygon &other) const {
  if (isFront(other) || isBehind(other)) return false;
  return EQUAL(plane_normal, other.plane_normal);
}

void Polygon::apply(glm::mat4 transform) {
  std::vector<glm::vec3> new_points;
  for(auto & p: points) {
    new_points.push_back(glm::vec3(transform * glm::vec4(p, 1.0f)));
  }
  points = new_points;
  plane_normal = glm::vec3(transform * glm::vec4(plane_normal, 0.0f));
}

void BSPNode::print(int indent, int index) {
  printf("%*sindex : %d\n", indent, " ", index);
  for(int i = 0; i < polygons.size(); ++i) {
    auto polygon = polygons[i];
    printf("%*s(%d\n", indent, " ", i);
    for(auto const& point: polygon.points) {
      printf("%*s%s\n", indent, " ", glm::to_string(point).c_str());
    }
    printf("%*s)\n", indent, " ");
  }
  if (front) {
    front->print(indent + 1, index * 2 + 1);
  }
  if (behind) {
    behind->print(indent + 1, index * 2 + 2);
  }
}

void BSPNode::apply(glm::mat4 transform) {
  if (!this) return;
  normal = glm::vec3(transform * glm::vec4(normal, 0.0f));
  point = glm::vec3(transform * glm::vec4(point, 1.0f));
  for(auto & p: polygons) {
    p.apply(transform);
  }
  front->apply(transform);
  behind->apply(transform);
}

BSPNode::BSPNode(std::vector<Polygon> polygons) {
  assert(!polygons.empty());
  // Choose a polygon P from the list
  Polygon& p = polygons[0];
  this->polygons.push_back(p);
  // For each other polygons in the list
  std::vector<Polygon> front, behind;
  this->normal = p.plane_normal;
  this->point = p.points[0];
  for (int i = 1; i < polygons.size(); ++i) {
    if (polygons[i].isFront(p)) front.push_back(polygons[i]);
    else if (polygons[i].isBehind(p)) behind.push_back(polygons[i]);
    else if (polygons[i].isOnSamePlane(p)) this->polygons.push_back(polygons[i]);
    else {
      // Split it into two polygons
      std::vector<Polygon> split = polygons[i].slice(p);
      if (split.size() == 2) {
        if (split[0].isFront(p) && split[1].isBehind(p)) {
          front.push_back(split[0]);
          behind.push_back(split[1]);
        } else if (split[1].isFront(p) && split[0].isBehind(p)) {
          front.push_back(split[1]);
          behind.push_back(split[0]);
        } else
          assert(false);
      }
      else {
        if (split[0].isFront(p)) front.push_back(split[0]);
        else if (split[0].isBehind(p)) behind.push_back(split[0]);
        else this->polygons.push_back(split[0]);
      }
    }
  }
  if (!front.empty()) {
    this->front = std::make_unique<BSPNode>(BSPNode(front));
  }
  else {
    this->front = nullptr;
  }
  if (!behind.empty()) {
    this->behind = std::make_unique<BSPNode>(BSPNode(behind));
  }
  else {
    this->behind = nullptr;
  }
}

bool BSPNode::isFront(glm::vec3 &p) const {
  // Is this p in front of (on normal side) this
  return ((float)glm::dot(p - point, normal) > EPSILON);
}

bool BSPNode::isBehind(glm::vec3 &p) const {
  // Is this p in front of (on normal side) this
  return ((float)glm::dot(p - point, normal) < -EPSILON);
}

std::vector<Polygon> BSPNode::getPolygons() {
  std::vector<Polygon> result;
  if (!this) return result;
  std::vector<Polygon> f = front->getPolygons();
  std::vector<Polygon> b = behind->getPolygons();
  result.reserve(polygons.size() + f.size() + b.size());
  result.insert(result.end(), polygons.begin(), polygons.end());
  result.insert(result.end(), f.begin(), f.end());
  result.insert(result.end(), b.begin(), b.end());
  return result;
}

BSPTree::BSPTree(std::vector<Polygon> polygons) {
  assert(!polygons.empty());
  root = std::make_unique<BSPNode>(BSPNode(polygons));
}

BSPTree::BSPTree(std::vector<glm::vec3> & vertices, glm::vec3 Kd, glm::vec3 Ka, glm::vec3 Ks, float n, glm::vec4 color) {
  assert(vertices.size() % 3 == 0);
  std::vector<Polygon> polygons;
  for (int i = 0; i < vertices.size() / 3; ++i) {
    std::vector<glm::vec3> points;
    for (int j = 0; j < 3; ++j) {
      points.push_back(vertices[i * 3 + j]);
    }
    try {
      polygons.push_back(Polygon(points, Kd, Ka, Ks, n, color));
    }
    catch (int _) {}
  }
  root = std::make_unique<BSPNode>(BSPNode(polygons));
}

void BSPTree::print() {
  root->print(0, 0);
}

void BSPTree::apply(glm::mat4 transform) {
  root->apply(transform);
}

std::vector<Polygon> BSPTree::getPolygons() {
  return root->getPolygons();
}
