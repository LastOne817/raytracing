#include "texture.h"

Texture::Texture(const char *imagepath) {
  FILE * file = fopen(imagepath, "rb");
  assert(file);

  unsigned char header[54];

  assert(fread(header, 1, 54, file) == 54);
  assert(header[0] == 'B' && header[1] == 'M');

  assert(*(int*)&(header[0x1E]) == 0);
  assert(*(int*)&(header[0x1C]) == 24);

  unsigned int dataPos = *(int*)&(header[0x0A]);
  unsigned int imageSize = *(int*)&(header[0x22]);
  width = *(int*)&(header[0x12]);
  height = *(int*)&(header[0x16]);

  if (imageSize == 0) imageSize = width * height * 3;
  if (dataPos == 0) dataPos = 54;

  data = new unsigned char[imageSize];
  fread(data, 1, imageSize, file);
  fclose(file);
}

Texture::~Texture() {
  delete [] data;
}

glm::vec3 Texture::getTexture(double u, double v) {
  int x = (int)((u + 1.0) / 2.0 * width);
  int y = (int)((v + 1.0) / 2.0 * height);
  unsigned char r = data[(y * width + x) * 3 + 2];
  unsigned char g = data[(y * width + x) * 3 + 1];
  unsigned char b = data[(y * width + x) * 3 + 0];
  return glm::vec3((int)r, (int)g, (int)b) / 255;
}

