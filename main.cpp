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
  double ambient;
  vec3 light;

  RandomShader(const Model &m) : model(m) {}

  virtual vec4 vertex(const int face, const int vert) {
    vec3 v = model.vert(face, vert);
    vec4 gl_position = ModelView * to_vec4(v, 1);
    tri[vert] = to_vec3(gl_position);
    return Perspective * gl_position;
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 barycentric) const {
    auto n = unit_vector(cross(tri[1] - tri[0], tri[2] - tri[0]));
    auto diffuse = std::fmax(0.0, dot(n, light));
    auto r = dot(n, light) * 2 * n - light;
    // v is always (0,0,1) because we build the camera on l,m,n and n is the
    // third basis vector
    auto e = 10;
    auto specular = std::fmax(0, std::pow(dot(r, vec3{0, 0, 1}), e));

    auto phong =
        std::fmin(1.0, barycentric.x * ambient + barycentric.y * diffuse +
                           barycentric.z * specular);
    TGAColor result;
    for (int ch = 0; ch < 3; ch++) {
      double value = color[ch] * phong;
      result[ch] = static_cast<std::uint8_t>(value);
    }
    return {false, result};
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
  constexpr vec3 light{1, 1, 1};
  const vec3 l = unit_vector(to_vec3(ModelView * to_vec4(light, 0)));

  for (int m = 1; m < argc; m++) {
    Model model(argv[m]);
    RandomShader shader(model);

    for (int f = 0; f < model.num_faces(); f++) {
      auto rnd = [] { return static_cast<std::uint8_t>(std::rand() % 256); };
      shader.color = {{rnd(), rnd(), rnd(), 255}};
      shader.ambient = 0.2;
      shader.light = l;

      Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1),
                       shader.vertex(f, 2)};
      rasterize(clip, shader, framebuffer);
    }
  }

  framebuffer.write_tga_file("renders/framebuffer.tga");

  return 0;
}
