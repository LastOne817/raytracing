// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <glm/glm.hpp>
#include <common/raytracing.h>

int main(void)
{
  World world;
  Sphere s1 = Sphere(glm::vec3(0.0f), 10.0,
                     glm::vec3(0.19125f, 0.0735f, 0.0225f),
                     glm::vec3(0.7038f, 0.27048f, 0.0828f),
                     glm::vec3(0.256777f, 0.137622f, 0.086014f),
                     90,
                     1.5, false, false);
  Sphere s2 = Sphere(glm::vec3(5.0f, 5.0f, -12.0f), 3.0,
                     glm::vec3(0.1745f, 0.01175f, 0.01175f),
                     glm::vec3(0.61424f, 0.04136f, 0.04136f),
                     glm::vec3(0.727811f, 0.626959f, 0.626959f),
                     78.6,
                     1.5, false, false);
  Sphere s3 = Sphere(glm::vec3(-30.0f, 0.0f, 20.0f), 12.0,
                     glm::vec3(0.1745f, 0.01175f, 0.01175f),
                     glm::vec3(0.61424f, 0.04136f, 0.04136f),
                     glm::vec3(0.727811f, 0.626959f, 0.626959f),
                     78.6,
                     1.5, false, false);
  auto vertices = std::vector<glm::vec3>{
      glm::vec3(100.0f, -10.0f, -100.0f),
      glm::vec3(-100.0f, -10.0f, -100.0f),
      glm::vec3(-100.0f, -10.0f, 100.0f),
      glm::vec3(100.0f, -10.0f, 100.0f),
  };
  Polygon floor = Polygon(vertices,
                          glm::vec3(0.4f),
                          glm::vec3(0.25f),
                          glm::vec3(0.774597f),
                          78.6f, 1.0, false, false);
  Light l1 = Light(glm::vec3(0.0f, 50.0f, -30.0f), 5000.0f);
  Light l2 = Light(glm::vec3(50.0f, 0.0f, -30.0f), 5000.0f);
  world.addObject((Object*)&s1);
  world.addObject((Object*)&s2);
  world.addObject((Object*)&s3);
  world.addObject((Object*)&floor);
  world.addLight(l1);
  world.addLight(l2);
  world.createImageFromView(glm::vec3(0.0f, 0.0f, -25.0f), glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0,1,0), 20, 1600, 1200);
  return 0;
}
