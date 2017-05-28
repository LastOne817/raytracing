#include "raytracing.h"

std::experimental::optional<Object*> World::intersect(Ray ray) const {
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

glm::vec3 World::trace(Ray ray, int depth) const {
  glm::vec3 background_color = glm::vec3(135.0 / 255, 206.0 / 255, 235.0 / 255);
  if (depth > DEPTH_MAX) return background_color;

  auto obj_test = intersect(ray);
  if (!obj_test) return background_color;
  Object* obj = obj_test.value();
  auto c = accumulateLightSource(obj, ray);
  double weightSum = 1.0;

  if (obj->reflective) {
    Ray reflect = obj->reflect(ray);
    c += trace(reflect, depth + 1) * obj->reflectWeight;
    weightSum += obj->reflectWeight;
  }
  if (obj->refractive) {
    Ray refract = obj->refract(ray);
    double refractWeight = 4.0f;
    c += trace(refract, depth + 1) * refractWeight;
    weightSum += refractWeight;
  }
  return c / weightSum;
}

glm::vec3 World::accumulateLightSource(Object* obj, Ray ray) const {
  glm::vec3 q = obj->intersect(ray).value();
  glm::vec3 c = obj->ambient;
  // glm::vec3 c = glm::vec3(0);
  for(auto & light: lights) {
    if (reachable(light, q)) {
      glm::vec3 L = glm::normalize(light.position - q);
      glm::vec3 N = obj->normalAt(q);
      double nl = glm::dot(N, L);
      double distance = glm::distance(light.position, q);
      if (nl > EPSILON) {
        c += obj->diffuse * light.power / (distance*distance) * nl;
      }
      glm::vec3 R = glm::dot(2.0 * L, N) * N - L;
      glm::vec3 V = glm::normalize(eye - q);
      if (glm::dot(R, V) > EPSILON) {
        c += obj->specular * light.power / (distance*distance) * pow(glm::dot(R, V), obj->gloss);
      }
    }
  }
  return c;
}

bool World::reachable(Light light, glm::vec3 target) const {
  Ray ray = Ray(target, light.position - target, 0.0);
  for(auto & object: objects) {
    auto p = object->intersect(ray);
    if (p && glm::distance(light.position, target) > glm::distance(target, p.value())) {
      return false;
    }
  }
  return true;
}

png_byte cut(double value) {
  if (value < 0.0) return 0;
  else if (value > 255) return 255;
  else return (png_byte)value;
}

void* fillImage(void* arg) {
  int x, y;
  struct thread_data* data;
  data = (struct thread_data *)arg;

  int progress = 0;

  for (y = data->startIndex ; y < data->startIndex + data->height / NTHREAD ; y++) {
    for (x=0 ; x < data->width ; x++) {
      glm::vec3 color = data->world->calculateColor(x, y, data->direction, data->right, data->up, data->width, data->height, data->view_width, data->view_height);
      data->image[y * data->width * 3 + x * 3 + 0] = cut(color.x * 255.0);
      data->image[y * data->width * 3 + x * 3 + 1] = cut(color.y * 255.0);
      data->image[y * data->width * 3 + x * 3 + 2] = cut(color.z * 255.0);
    }
    if ((int)((y - data->startIndex) * 20.0 / (data->height / NTHREAD)) > progress) {
      progress = (int)((y - data->startIndex) * 20.0 / (data->height / NTHREAD));
      printf("Thread %d:\t%d%%\tdone...\n", data->id, progress * 5);
    }
  }
  pthread_exit(NULL);
}

void World::createImageFromView(glm::vec3 eye, glm::vec3 direction, glm::vec3 up, double view_width, int width, int height) {
  assert(height % NTHREAD == 0);
  this->eye = eye;
  FILE *fp = fopen("./result.png", "wb");
  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));
  png_init_io(png_ptr, fp);

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

  // Write image data
  double view_height = view_width / width * height;
  glm::vec3 right = glm::normalize(glm::cross(direction, up));
  pthread_t threads[NTHREAD];
  struct thread_data data[NTHREAD];
  void* status;

  png_byte* image = (png_byte*)malloc(sizeof(png_byte) * 3 * height * width);

  for(int t = 0; t < NTHREAD; t++) {
    data[t].id = t;
    data[t].startIndex = t * height / NTHREAD;
    data[t].world = this;
    data[t].width = width;
    data[t].height = height;
    data[t].view_width = view_width;
    data[t].view_height = view_height;
    data[t].direction = direction;
    data[t].right = right;
    data[t].up = up;
    data[t].image = image;
    pthread_create(&threads[t], NULL, fillImage, (void*)(&data[t]));
  }

  for(int t = 0; t < NTHREAD; t++) {
    pthread_join(threads[t], &status);
  }

  png_bytep row = NULL;

  // Allocate memory for one row (3 bytes per pixel - RGB)
  row = (png_bytep) malloc(3 * width * sizeof(png_byte));
  for (int y = 0; y < height; ++y) {
    png_write_row(png_ptr, &image[y * width * 3]);
  }

  // End write
  png_write_end(png_ptr, NULL);
  png_write_info(png_ptr, info_ptr);

  if (fp != NULL) fclose(fp);
  if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  if (row != NULL) free(row);
}

glm::vec3 World::calculateColor(int x, int y, glm::vec3 direction, glm::vec3 right, glm::vec3 up, int width, int height, double view_width, double view_height) const {
  bool distributed = true;
  glm::vec3 color;
  if (distributed) {
    color = glm::vec3(0);
    for (int i = -3; i <= 3; ++i) {
      for (int j = -3; j <= 3; ++j) {
        glm::vec3 target = eye + direction +
                           right * ((double) x / width * 2.0 - 1.0) * view_width +
                           up * (1.0 - (double) y / height * 2.0) * view_height;
        glm::vec3 dist_eye = eye + right * (double) i * 0.5f + up * (double) j * 0.5f;
        Ray ray = Ray(dist_eye, target - dist_eye, 1.0);
        glm::vec3 c = trace(ray, 0);
        color += c;
      }
    }
    color /= 49.0f;
  }
  else {
    glm::vec3 target = eye + direction +
                       right * ((double) x / width * 2.0 - 1.0) * view_width +
                       up * (1.0 - (double) y / height * 2.0) * view_height;
    Ray ray = Ray(eye, target - eye, 1.0);
    color = trace(ray, 0);
  }
  return color;
}
