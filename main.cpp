#include "geometry.h"
#include "model.h"

#include <sstream>

#include "tgaimage.h"

#include <algorithm>
#include <filesystem>

constexpr TGAColor white = {{255, 255, 255, 255}}; // attention, BGRA order
constexpr TGAColor green = {{0, 255, 0, 255}};
constexpr TGAColor red = {{0, 0, 255, 255}};
constexpr TGAColor blue = {{255, 128, 64, 255}};
constexpr TGAColor yellow = {{0, 200, 255, 255}};

mat<4, 4> ModelView, Viewport, Perspective;

void viewport(const int x, const int y, const int w, const int h) {
  Viewport = {{{w / 2.0, 0, 0, x + w / 2.0},
               {0, h / 2.0, 0, y + h / 2.0},
               {0, 0, 1, 0},
               {0, 0, 0, 1}}};
}

void perspective(const double f) {
  Perspective = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, f}}};
}

void lookat(const vec3 &eye, const vec3 &center, const vec3 &up) {
  vec n = unit_vector(eye - center);
  vec l = unit_vector(cross(up, n));
  vec m = unit_vector(cross(l, n));

  ModelView = mat<4, 4>{{{l.x, l.y, l.z, 0},
                         {m.x, m.y, m.z, 0},
                         {n.x, n.y, n.z, 0},
                         {0, 0, 0, 1}

              }} *
              mat<4, 4>{{{1, 0, 0, -center.x},
                         {0, 1, 0, -center.y},
                         {0, 0, 1, center.z},
                         {0, 0, 0, 1}}};
}

void rasterize(vec4 clip[3], TGAImage &framebuffer, TGAColor color_in[3],
               std::vector<std::vector<double>> &depth_buffer,
               TGAImage &zbuffer) {
  vec4 ndc[3] = {clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w};
  vec2 screen[3] = {to_vec2(Viewport * ndc[0]), to_vec2(Viewport * ndc[1]),
                    to_vec2(Viewport * ndc[2])};
  mat<3, 3> ABC = {{{screen[0].x, screen[0].y, 1.0},
                    {screen[1].x, screen[1].y, 1.0},
                    {screen[2].x, screen[2].y, 1.0}}};
  if (determinant(ABC) < 1)
    return;

  auto [bbminx, bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
  auto [bbminy, bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y});

#pragma omp parallel for
  // Clamp to [0, width), [0, height)
  for (int x = std::max<int>(bbminx, 0);
       x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++) {
    for (int y = std::max<int>(bbminy, 0);
         y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++) {
      vec3 barycentric_weights =
          inverse(transpose(ABC)) *
          vec3(static_cast<double>(x), static_cast<double>(y), 1.0);

      if (barycentric_weights.x < 0 || barycentric_weights.y < 0 ||
          barycentric_weights.z < 0) {
        continue;
      }
      double z = dot(barycentric_weights, vec3(ndc[0].z, ndc[1].z, ndc[2].z));
      if (z > depth_buffer[y][x]) {

        TGAColor color;
        // Gradient
        for (int ch = 0; ch < 3; ch++) {
          color[ch] = barycentric_weights.x * color_in[0][ch] +
                      barycentric_weights.y * color_in[1][ch] +
                      barycentric_weights.z * color_in[2][ch];
        }
        framebuffer.set(x, y, color);
        depth_buffer[y][x] = z;
        zbuffer.set(x, y, color);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
    return 1;
  }

  constexpr int aspect_ratio = 1;
  constexpr int width = 1200;
  constexpr int height = width / aspect_ratio;
  constexpr vec3 eye{-1, 0, 2};
  constexpr vec3 center{0, 0, 0};
  const vec3 up{0, 1, 0};

  lookat(eye, center, up);
  perspective(length((eye - center)));
  viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

  Model model(argv[1]);
  TGAImage framebuffer(width, height, TGAImage::RGB);
  std::vector<std::vector<double>> depth_buffer(
      height,
      std::vector<double>(width, std::numeric_limits<double>::lowest()));
  TGAImage zbuffer(width, height, std::numeric_limits<double>::lowest());

  for (int i = 0; i < model.num_faces(); i++) {
    vec4 clip[3];
    for (int d : {0, 1, 2}) {
      vec3 v = model.vert(i, d);
      clip[d] = Perspective * ModelView * to_vec4(v, 1);
    }
    TGAColor random_color[3];

    // Build colors
    for (int g = 0; g < 3; g++) {
      for (int v = 0; v < 3; v++) {
        random_color[g][v] = std::rand() % 256;
      }
    }
    rasterize(clip, framebuffer, random_color, depth_buffer, zbuffer);
  }

  framebuffer.write_tga_file("renders/framebuffer.tga");
  zbuffer.write_tga_file("renders/zbuffer.tga");

  return 0;
}
