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

void triangle(int ax, int ay, int bx, int by, int cx, int cy,
              TGAImage &framebuffer, TGAColor color) {
  if (ay > by) {
    std::swap(ax, bx);
    std::swap(ay, by);
  }
  if (ay > cy) {
    std::swap(ax, cx);
    std::swap(ay, cy);
  }
  if (by > cy) {
    std::swap(bx, cx);
    std::swap(by, cy);
  }
  line(ax, ay, bx, by, framebuffer, color);
  line(bx, by, cx, cy, framebuffer, color);
  line(cx, cy, ax, ay, framebuffer, color);

  if (ay == cy) {
    return;
  }

  float next_left = ax;
  float next_right = ax;
  float dx_ac = static_cast<float>((cx - ax)) / (cy - ay);
  float dx_ab = static_cast<float>((bx - ax)) / (by - ay);
  float dx_bc = static_cast<float>((cx - bx)) / (cy - by);

  for (int y = ay; y <= cy; y++) {
    int x_start = std::round(std::min(next_left, next_right));
    int x_end = std::round(std::max(next_left, next_right));

    for (int x = x_start; x <= x_end; x++) {
      framebuffer.set(std::round(x), y, color);
    }

    if (y < by) {
      next_left += dx_ac;
      next_right += dx_ab;
    } else {
      next_left += dx_ac;
      next_right += dx_bc;
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
