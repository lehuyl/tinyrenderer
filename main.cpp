#include "geometry.h"
#include "model.h"

#include <sstream>

#include "tgaimage.h"

#include <filesystem>

constexpr TGAColor white = {{255, 255, 255, 255}}; // attention, BGRA order
constexpr TGAColor green = {{0, 255, 0, 255}};
constexpr TGAColor red = {{0, 0, 255, 255}};
constexpr TGAColor blue = {{255, 128, 64, 255}};
constexpr TGAColor yellow = {{0, 200, 255, 255}};

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
  vertex.z = (vertex.z + 1) * width / 2;

  return vertex;
}

double signed_triangle_area(double ax, double ay, double bx, double by,
                            double cx, double cy) {
  return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) +
               (ay - cy) * (ax + cx));
}

void triangle(vec3 a, vec3 b, vec3 c, TGAImage &framebuffer,
              TGAColor color_in[3],
              std::vector<std::vector<double>> &depth_buffer) {
  // clamp the bounding box to the framebuffer
  double bbminx = std::fmax(std::fmin(std::fmin(a.x, b.x), c.x), 0.0);
  double bbminy = std::fmax(std::fmin(std::fmin(a.y, b.y), c.y), 0.0);
  double bbmaxx = std::fmin(std::fmax(std::fmax(a.x, b.x), c.x), width - 1);
  double bbmaxy = std::fmin(std::fmax(std::fmax(a.y, b.y), c.y), height - 1);
  double total_area = signed_triangle_area(a.x, a.y, b.x, b.y, c.x, c.y);
  if (total_area < 1)
    return; // backface culling + discarding triangles that cover less than 1
            // pixel

#pragma omp parallel for
  for (int x = bbminx; x <= bbmaxx; x++) {
    for (int y = bbminy; y <= bbmaxy; y++) {
      double alpha =
          signed_triangle_area(x, y, b.x, b.y, c.x, c.y) / total_area;
      double beta = signed_triangle_area(a.x, a.y, x, y, c.x, c.y) / total_area;
      double gamma =
          signed_triangle_area(a.x, a.y, b.x, b.y, x, y) / total_area;

      if (alpha < 0 || beta < 0 || gamma < 0) {
        continue;
      }
      // if (alpha > .1 && beta > .1 && gamma > .1) {
      //   continue;
      // }
      double z = alpha * a.z + beta * b.z + gamma * c.z;
      if (z > depth_buffer[y][x]) {
        TGAColor color;
        // Gradient
        for (int ch = 0; ch < 3; ch++) {
          color[ch] = alpha * color_in[0][ch] + beta * color_in[1][ch] +
                      gamma * color_in[2][ch];
        }
        framebuffer.set(x, y, color);
        depth_buffer[y][x] = z;
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
    return 1;
  }

  Model model(argv[1]);
  TGAImage framebuffer(width, height, TGAImage::RGB);
  std::vector<std::vector<double>> depth_buffer(
      height,
      std::vector<double>(width, std::numeric_limits<double>::lowest()));

  for (int i = 0; i < model.num_faces(); i++) {
    auto a = project(rotate(model.vert(i, 0)));
    auto b = project(rotate(model.vert(i, 1)));
    auto c = project(rotate(model.vert(i, 2)));
    TGAColor random_color[3];

    // Build colors
    for (int g = 0; g < 3; g++) {
      for (int v = 0; v < 3; v++) {
        random_color[g][v] = std::rand() % 256;
      }
    }
    triangle(a, b, c, framebuffer, random_color, depth_buffer);
  }

  framebuffer.write_tga_file("renders/framebuffer.tga");

  return 0;
}
