#include <iostream>
#include <sstream>

#include "tgaimage.h"

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

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

struct Vertex {
  float x, y, z;
};

struct Corner {
  int v;  // Vertex
  int vt; // Texture
  int vn; // Normal
};

struct Face {
  Corner corners[3];
};

Vertex map_to_2d_space(Vertex vertex, int width, int height) {
  vertex.x = (vertex.x + 1) * width / 2;
  vertex.y = (vertex.y + 1) * height / 2;
  vertex.z = (vertex.z + 1) / 2;

  return vertex;
}

int main(int argc, char **argv) {
  constexpr int aspect_ratio = 1;
  constexpr int width = 1200;
  constexpr int height = width / aspect_ratio;
  TGAImage framebuffer(width, height, TGAImage::RGB);

  std::ifstream file("obj/diablo3_pose/diablo3_pose.obj");

  if (!file.is_open()) {
    std::cerr << "Error opening file" << std::endl;
    return -1;
  }

  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  char slash;

  std::string row;
  while (std::getline(file, row)) {
    std::istringstream iss(row);
    std::string type;
    iss >> type;
    if (type == "v") {
      Vertex vertex;
      iss >> vertex.x >> vertex.y >> vertex.z;

      vertices.push_back(vertex);
    } else if (type == "f") {
      Face face;
      for (int i = 0; i < 3; i++) {
        Corner corner;
        iss >> corner.v >> slash >> corner.vt >> slash >> corner.vn;
        corner.v--;
        corner.vt--;
        corner.vn--;
        face.corners[i] = corner;
      }
      faces.push_back(face);
    }
  }

  for (const Face &face : faces) {
    Vertex a = map_to_2d_space(vertices[face.corners[0].v], width, height);
    Vertex b = map_to_2d_space(vertices[face.corners[1].v], width, height);
    Vertex c = map_to_2d_space(vertices[face.corners[2].v], width, height);

    line(a.x, a.y, b.x, b.y, framebuffer, red);
    line(a.x, a.y, c.x, c.y, framebuffer, red);
    line(b.x, b.y, c.x, c.y, framebuffer, red);
  }

  framebuffer.write_tga_file("framebuffer.tga");

  return 0;
}
