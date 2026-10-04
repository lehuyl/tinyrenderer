#include "geometry.h"
#include "model.h"
#include "our_gl.h"

#include <sstream>

#include "tgaimage.h"

#include <algorithm>

extern mat<4, 4> ModelView, Perspective;
extern std::vector<double> zbuffer;

struct PhongShader : IShader {
  const Model &model;
  TGAColor color = {};
  TGAColor specular_color = {};
  vec3 tri[3]; // triangle in eye coordinates
  vec3 tri_normal[3];
  vec3 l;

  PhongShader(const vec3 &light, const Model &m) : model(m) {
    l = unit_vector(to_vec3(ModelView * to_vec4(light, 0)));
  }

  virtual vec4 vertex(const int face, const int vert) {
    vec3 v = model.vert(face, vert);
    vec4 gl_position = ModelView * to_vec4(v, 1);
    tri[vert] = to_vec3(gl_position);

    vec3 n = model.normal(face, vert);
    vec4 gl_normal = ModelView * to_vec4(n, 0);
    tri_normal[vert] = to_vec3(gl_normal);

    return Perspective * gl_position;
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 barycentric) const {
    auto n = unit_vector(barycentric.x * tri_normal[0] +
                         barycentric.y * tri_normal[1] +
                         barycentric.z * tri_normal[2]);
    auto diffuse = std::fmax(0.0, dot(n, l));
    auto r = dot(n, l) * 2 * n - l;
    // v is always (0,0,1) because we build the camera on l,m,n and n is the
    // third basis vector
    auto e = 88;
    auto specular = std::pow(std::fmax(0.0, dot(r, vec3{0, 0, 1})), e);
    double ambient = 0.3;
    TGAColor result;
    for (int ch = 0; ch < 3; ch++) {
      double value = color[ch] * (ambient + 0.6 * diffuse) +
                     specular_color[ch] * 0.9 * specular;
      result[ch] = static_cast<std::uint8_t>(std::fmin(255.0, value));
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
  TGAColor grey{{128, 128, 128, 255}};

  TGAColor white{{255, 255, 255, 255}};

  lookat(eye, center, up);
  init_perspective(length((eye - center)));
  init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);
  init_zbuffer(width, height);
  TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 255, 209});
  constexpr vec3 light{1, 1, 1};

  for (int m = 1; m < argc; m++) {
    Model model(argv[m]);
    PhongShader shader(light, model);

    for (int f = 0; f < model.num_faces(); f++) {
      shader.color = grey;
      shader.specular_color = white;

      Triangle clip = {shader.vertex(f, 0), shader.vertex(f, 1),
                       shader.vertex(f, 2)};
      rasterize(clip, shader, framebuffer);
    }
  }

  framebuffer.write_tga_file("renders/framebuffer.tga");

  return 0;
}
