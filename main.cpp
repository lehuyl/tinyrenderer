#include "geometry.h"
#include "model.h"
#include "our_gl.h"

#include <sstream>

#include "tgaimage.h"

#include <algorithm>

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;

struct RandomShader : IShader {
  const Model &model;
  TGAColor color = {};
  vec3 tri[3]; // triangle in eye coordinates

  RandomShader(const Model &m) : model(m) {}

  virtual vec4 vertex(const int face, const int vert) {
    vec3 v = model.vert(face, vert);
    vec4 gl_position = ModelView * to_vec4(v, 1);
    tri[vert] = to_vec3(gl_position);
    return Perspective * gl_position;
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const {
    return {false, color};
  }
};

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
  init_perspective(length((eye - center)));
  init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
  init_zbuffer(width, height);
  TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 255, 209});

  for (int m = 1; m < argc; m++) {
    Model model(argv[m]);
    RandomShader shader(model);

    for (int f = 0; f < model.num_faces(); f++) {
      auto rnd = [] { return static_cast<std::uint8_t>(std::rand() % 256); };
      shader.color = {{rnd(), rnd(), rnd(), 255}};
      Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1),
                       shader.vertex(f, 2)};
      rasterize(clip, shader, framebuffer);
    }
  }

  framebuffer.write_tga_file("renders/framebuffer.tga");

  return 0;
}
