#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include "color.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 original;
};

struct Fragment {
  uint16_t x;      
  uint16_t y;      
  double z;  // zbuffer
  Color color; // r, g, b values for color
  float intensity;  // light intensity
  glm::vec3 original;
};

struct FragColor {
  Color color;
  double z; // instead of z buffer
};

