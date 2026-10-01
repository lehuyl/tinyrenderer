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
constexpr int width = 1200;
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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
    return 1;
  }

  Model model(argv[1]);

  TGAImage framebuffer(width, height, TGAImage::RGB);

  for (int i = 0; i < model.num_faces(); i++) {
    auto a = project(model.vert(i, 0));
    auto b = project(model.vert(i, 1));
    auto c = project(model.vert(i, 2));

    line(a.x, a.y, b.x, b.y, framebuffer, red);
    line(a.x, a.y, c.x, c.y, framebuffer, red);
    line(b.x, b.y, c.x, c.y, framebuffer, red);
  }

  for (int i = 0; i < model.num_vertices(); i++) {
    auto v = project(model.vert(i));
    framebuffer.set(v.x, v.y, white);
  }

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
}
