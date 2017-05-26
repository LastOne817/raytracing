#ifndef GRAPHICS_RAYTRACING_H
#define GRAPHICS_RAYTRACING_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "object.h"

#include <vector>
#include <png.h>

#define DEPTH_MAX 5

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
    std::experimental::optional<Object*> intersect(Ray ray);
    glm::vec3 accumulateLightSource(Object* obj, Ray ray);
    bool reachable(Light light, glm::vec3 target);
    glm::vec3 eye;
public:
    glm::vec3 trace(Ray ray, int depth);
    void addObject(Object* obj) { objects.push_back(obj); }
    void addLight(Light& light) { lights.push_back(light); }
    void createImageFromView(glm::vec3 eye, glm::vec3 direction, glm::vec3 up, double view_width, int width, int height);
};

std::experimental::optional<Object*> World::intersect(Ray ray) {
  glm::vec3 closest = glm::vec3(INFINITY);
  std::experimental::optional<Object*> result = {};
  for(auto & object: objects) {
    std::experimental::optional<glm::vec3> p = object->intersect(ray);
    if (p && glm::distance(p.value(), ray.origin) < glm::distance(closest, ray.origin)) {
      closest = p.value();
      result = object;
    }
  }
  return result;
}

glm::vec3 World::trace(Ray ray, int depth) {
  glm::vec3 background_color = glm::vec3(135.0 / 255, 206.0 / 255, 235.0 / 255);
  if (depth > DEPTH_MAX) return background_color;

  auto obj_test = intersect(ray);
  if (!obj_test) return background_color;
  Object* obj = obj_test.value();
  auto c = accumulateLightSource(obj, ray);

  if (obj->reflective) {
    Ray reflect = obj->reflect(ray);
    c += trace(reflect, depth + 1);
  }
  if (obj->refractive) {
    Ray refract = obj->refract(ray);
    c += trace(refract, depth + 1);
  }
  return c;
}

glm::vec3 World::accumulateLightSource(Object* obj, Ray ray) {
  glm::vec3 q = obj->intersect(ray).value();
  glm::vec3 c = obj->ambient;
  for(auto & light: lights) {
    if (reachable(light, q)) {
      glm::vec3 L = glm::normalize(light.position - q);
      glm::vec3 N = obj->normalAt(q);
      double nl = glm::dot(N, L);
      double distance = glm::distance(light.position, q);
      if (nl > 0.0) {
        c += obj->diffuse * light.power / (distance*distance) * nl;
      }
      glm::vec3 R = glm::dot(2.0 * L, N) * N - L;
      glm::vec3 V = glm::normalize(eye - q);
      if (glm::dot(R, V) > 0.0) {
        c += obj->specular * light.power / (distance*distance) * pow(glm::dot(R, V), obj->gloss);
      }
    }
  }
  return c;
}

bool World::reachable(Light light, glm::vec3 target) {
  Ray ray = Ray(target, light.position - target);
  for(auto & object: objects) {
    auto p = object->intersect(ray);
    if (p && glm::distance(light.position, target) > glm::distance(light.position, p.value())) return false;
  }
  return true;
}

int cut(double value) {
  if (value < 0.0) return 0;
  else if (value > 255) return 255;
  else return (int)value;
}

void World::createImageFromView(glm::vec3 eye, glm::vec3 direction, glm::vec3 up, double view_width, int width, int height) {
  this->eye = eye;
  FILE *fp = fopen("./result.png", "wb");
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));
  png_init_io(png_ptr, fp);
  png_bytep row = NULL;

  // Write header (8 bit colour depth)
  png_set_IHDR(png_ptr, info_ptr, width, height,
               8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  // Set title
  png_text title_text;
  title_text.compression = PNG_TEXT_COMPRESSION_NONE;
  title_text.key = (png_charp)"Title";
  title_text.text = (png_charp)"Graphics";
  png_set_text(png_ptr, info_ptr, &title_text, 1);

  png_write_info(png_ptr, info_ptr);

  // Allocate memory for one row (3 bytes per pixel - RGB)
  row = (png_bytep) malloc(3 * width * sizeof(png_byte));

  // Write image data
  int x, y;
  double view_height = view_width / width * height;
  glm::vec3 right = glm::normalize(glm::cross(direction, up));
  int progress = 0;
  for (y=0 ; y<height ; y++) {
    for (x=0 ; x<width ; x++) {
      glm::vec3 color = glm::vec3(0);
      for (int i = -3; i <= 3; ++i) {
        for (int j = -3; j <= 3; ++j) {
          glm::vec3 target = eye + direction +
                             right * ((double)x / width * 2.0 - 1.0) * view_width +
                             up * (1.0 - (double)y / height * 2.0) * view_height;
          glm::vec3 dist_eye = eye + right * (double)i * 0.2f + up * (double)j * 0.2f;
          Ray ray = Ray(dist_eye, target - dist_eye);
          glm::vec3 c = trace(ray, 0);
          color += c;
        }
      }
      color /= 49.0f;
      row[x * 3 + 0] = cut(color.x * 255.0);
      row[x * 3 + 1] = cut(color.y * 255.0);
      row[x * 3 + 2] = cut(color.z * 255.0);
    }
    png_write_row(png_ptr, row);
    if ((int)((double)y * 100.0 / height) > progress) {
      progress = (int)((double)y * 100.0 / height);
      printf("%d%% done...\n", progress);
    }
  }

  // End write
  png_write_end(png_ptr, NULL);
  png_write_info(png_ptr, info_ptr);

  if (fp != NULL) fclose(fp);
  if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  if (row != NULL) free(row);
}

#endif //GRAPHICS_RAYTRACING_H
