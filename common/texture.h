#ifndef GRAPHICS_TEXTURE_H
#define GRAPHICS_TEXTURE_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Texture {
private:
    unsigned char* data;
    unsigned int width, height;
public:
    Texture(const char* imagepath);
    ~Texture();
    glm::vec3 getTexture(double u, double v);
};

#endif //GRAPHICS_TEXTURE_H
