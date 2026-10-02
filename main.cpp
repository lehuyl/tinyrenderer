#include "geometry.h"
#include "model.h"

#include <sstream>

#include "tgaimage.h"

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

constexpr int aspect_ratio = 1;
constexpr int width = 128;
constexpr int height = width / aspect_ratio;

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer,
          TGAColor color) {
  bool steep = std::abs(ax - bx) < std::abs(ay - by);
  if (steep) {
    std::swap(ax, ay);
    std::swap(bx, by);
  }

  if (ax > bx) {
    std::swap(ax, bx);
    std::swap(ay, by);
  }

  int y = ay;
  int ierror = 0;
  for (int x = ax; x <= bx; x++) {
    if (steep) {
      framebuffer.set(y, x, color);
    } else {
      framebuffer.set(x, y, color);
    }
    ierror += 2 * std::abs(by - ay);
    if (ierror > bx - ax) {
      y += by > ay ? 1 : -1;
      ierror -= 2 * (bx - ax);
    }
  }
}

vec3 project(vec3 vertex) {
  vertex.x = (vertex.x + 1) * width / 2;
  vertex.y = (vertex.y + 1) * height / 2;
  vertex.z = (vertex.z + 1) / 2;

  return vertex;
}

struct vec2 {
  float x, y;
};

inline float get_determinant(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

bool inside(int ax, int ay, int bx, int by, int cx, int cy, int x, int y) {
  vec2 v0(bx - ax, by - ay);
  vec2 v1(cx - ax, cy - ay);
  vec2 vp(x - ax, y - ay);

  auto determinant = get_determinant(v0, v1);
  auto alpha = get_determinant(vp, v1) / determinant;
  auto beta = get_determinant(v0, vp) / determinant;

  // third coefficient is 1 - alpha - beta
  if (alpha < 0 || beta < 0 || alpha + beta > 1) {
    return false;
  } else {
    return true;
  }
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy,
              TGAImage &framebuffer, TGAColor color) {
  int bbminx = std::min(std::min(ax, bx), cx);
  int bbminy = std::min(std::min(ay, by), cy);
  int bbmaxx = std::max(std::max(ax, bx), cx);
  int bbmaxy = std::max(std::max(ay, by), cy);

#pragma omp parallel for
  for (int x = bbminx; x <= bbmaxx; x++) {
    for (int y = bbminy; y <= bbmaxy; y++) {
      if (inside(ax, ay, bx, by, cx, cy, x, y)) {
        framebuffer.set(x, y, color);
      }
    }
  }
}

int main(int argc, char **argv) {
  TGAImage framebuffer(width, height, TGAImage::RGB);
  triangle(7, 45, 35, 100, 45, 60, framebuffer, red);
  triangle(120, 35, 90, 5, 45, 110, framebuffer, white);
  triangle(115, 83, 80, 90, 85, 120, framebuffer, green);

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
}
