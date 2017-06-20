// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <glm/glm.hpp>
#include <common/raytracing.h>
#include <common/objloader.hpp>

int main(void)
{
  World world;
  Sphere s1 = Sphere(glm::vec3(0.0f), 10.0,
                     glm::vec3(0.1f),
                     glm::vec3(0.7f),
                     glm::vec3(0.1f),
                     90,
                     1.05, false, true);
  Sphere s2 = Sphere(glm::vec3(5.0f, 5.0f, -12.0f), 3.0,
                     glm::vec3(0.0215f, 0.1745f, 0.0215f),
                     glm::vec3(0.07568f, 0.61424f, 0.07568f),
                     glm::vec3(0.633f, 0.727811f, 0.633f),
                     78.6,
                     1.5, true, false);
  Sphere s3 = Sphere(glm::vec3(-10.0f, 2.0f, 20.0f), 12.0,
                     glm::vec3(0.1745f, 0.01175f, 0.01175f),
                     glm::vec3(0.61424f, 0.04136f, 0.04136f),
                     glm::vec3(0.727811f, 0.626959f, 0.626959f),
                     78.6,
                     1.5, true, false);
  Sphere s4 = Sphere(glm::vec3(-10.0f, 2.0f, -20.0f), 12.0,
                     glm::vec3(0),
                     glm::vec3(0.1),
                     glm::vec3(0),
                     1.0,
                     1.5, false, false);
  auto vertices = std::vector<glm::vec3>{
      glm::vec3(50.0f, -10.0f, -50.0f),
      glm::vec3(-50.0f, -10.0f, -50.0f),
      glm::vec3(-50.0f, -10.0f, 50.0f),
      glm::vec3(50.0f, -10.0f, -50.0f),
      glm::vec3(-50.0f, -10.0f, 50.0f),
      glm::vec3(50.0f, -10.0f, 50.0f),
  };
  Polygon floor = Polygon(vertices,
                          glm::vec3(0.4f),
                          glm::vec3(0.25f),
                          glm::vec3(0.774597f),
                          78.6f, 1.0, true, false);
  auto mirror1_vertices = std::vector<glm::vec3>{
      glm::vec3(50.0f, -10.0f, 50.0f),
      glm::vec3(50.0f, 50.0f, 50.0f),
      glm::vec3(-50.0f, 50.0f, 50.0f),
      glm::vec3(50.0f, -10.0f, 50.0f),
      glm::vec3(-50.0f, 50.0f, 50.0f),
      glm::vec3(-50.0f, -10.0f, 50.0f),
  };
  Polygon mirror1 = Polygon(mirror1_vertices,
                            glm::vec3(0.0),
                            glm::vec3(0.0),
                            glm::vec3(0.0),
                            78.6f, 1.0, true, false);
  ((Object*)&mirror1)->reflectWeight = 10.0;
  auto mirror2_vertices = std::vector<glm::vec3>{
      glm::vec3(-50.0f, -10.0f, 50.0f),
      glm::vec3(-50.0f, 50.0f, 50.0f),
      glm::vec3(-50.0f, 50.0f, -50.0f),
      glm::vec3(-50.0f, -10.0f, 50.0f),
      glm::vec3(-50.0f, 50.0f, -50.0f),
      glm::vec3(-50.0f, -10.0f, -50.0f),
  };
  Polygon mirror2 = Polygon(mirror2_vertices,
                            glm::vec3(0.0),
                            glm::vec3(0.0),
                            glm::vec3(0.0),
                            78.6f, 1.0, true, false);

  ((Object*)&mirror2)->reflectWeight = 10.0;

  std::vector<glm::vec3> polygon_vertices;
  std::vector<glm::vec3> polygon_normals;
  assert(loadOBJ("/home/lastone817/raytracing/hw5/polygon.obj", polygon_vertices, polygon_normals));
  std::vector<glm::vec3> scale_vertices;
  for(auto const & p : polygon_vertices) {
    glm::mat4 transform = glm::translate(glm::vec3(40.0, 0.0, 0.0)) * glm::scale(glm::vec3(3.0));
    scale_vertices.push_back(glm::vec3(transform * glm::vec4(p, 1.0)));
  }
  Polygon p1 = Polygon(scale_vertices, polygon_normals,
                       glm::vec3(0.1745f, 0.01175f, 0.01175f),
                       glm::vec3(0.61424f, 0.04136f, 0.04136f),
                       glm::vec3(0.727811f, 0.626959f, 0.626959f),
                       10.0, 1.0, true, false);

  Texture texture = Texture("/home/lastone817/raytracing/hw5/texture.bmp");
  Texture bumpmap = Texture("/home/lastone817/raytracing/hw5/normal.bmp");
  Texture crystal = Texture("/home/lastone817/raytracing/hw5/crystal.bmp");

  ((Object*)&s4)->texture = &texture;
  ((Object*)&s4)->bumpmap = &bumpmap;
  ((Object*)&s3)->bumpmap = &crystal;

  world.addObject((Object*)&s1);
  world.addObject((Object*)&s2);
  world.addObject((Object*)&s3);
  world.addObject((Object*)&s4);
  world.addObject((Object*)&p1);
  world.addObject((Object*)&floor);
  world.addObject((Object*)&mirror1);
  world.addObject((Object*)&mirror2);

  for (int i = -3; i <= 3; ++i) {
    for (int j = -3; j <= 3; ++j) {
      Light l3 = Light(glm::vec3(0.0f + i * 5.0, 50.0f, 0.0f + j * 5.0), 5000.0f / 49);
      world.addLight(l3);
    }
  }
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      for (int k = -1; k <= 1; ++k) {
        Light l2 = Light(glm::vec3(-40.0f + i * 5.0, 20.0f + j * 5.0, -30.0f + k * 5.0), 1000.0 / 9);
        world.addLight(l2);
      }
    }
  }
  Light l2 = Light(glm::vec3(0.0f + 0 * 5.0, 50.0f + 0 * 5.0, 0.0f + 0 * 5.0), 5000.0);
  world.addLight(l2);
  Light l3 = Light(glm::vec3(0.0f + 0 * 5.0, 50.0f, 0.0f + 0 * 5.0), 5000.0f);
  world.addLight(l3);
  world.createImageFromView(glm::vec3(140.0f, 40.0f, -140.0f), glm::vec3(-140.0f, -40.0f, 140.0f), glm::vec3(0,1,0), 80, 3200, 2400);
  return 0;
}
