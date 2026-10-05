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
  vec3 tri_tex[3];
  vec3 tri_normal[3];

  vec3 l;
  mat<4, 4> inverse_transpose_ModelView = inverse(transpose(ModelView));

  PhongShader(const vec3 &light, const Model &m) : model(m) {
    l = unit_vector(to_vec3(ModelView * to_vec4(light, 0)));
  }

  virtual vec4 vertex(const int face, const int vert) {
    vec3 v = model.vert(face, vert);
    vec4 gl_position = ModelView * to_vec4(v, 1);
    tri[vert] = to_vec3(gl_position);

    vec t = model.tex(face, vert);
    tri_tex[vert] = to_vec3(t);

    vec3 n = model.normal(face, vert);
    vec4 gl_normal = inverse_transpose_ModelView * to_vec4(n, 0);
    tri_normal[vert] = unit_vector(to_vec3(gl_normal));

    return Perspective * gl_position;
  }

  static TGAColor sample(const TGAImage &image, const vec3 &uv) {
    return image.get(uv.x * image.width(), uv.y * image.height());
  }

  vec3 get_normal_from_nm_tga(const TGAImage &mapping, const vec3 &uv) const {
    TGAColor normal_pixel = sample(mapping, uv);
    auto x = (normal_pixel[2] / 255.0 * 2) - 1;
    auto y = (normal_pixel[1] / 255.0 * 2) - 1;
    auto z = (normal_pixel[0] / 255.0 * 2) - 1;
    auto normal =
        unit_vector(to_vec3((inverse_transpose_ModelView * vec4{x, y, z, 0})));
    return normal;
  }

  vec3 get_normal_from_nm_tangent_tga(const TGAImage &mapping, const vec3 &uv,
                                      const vec3 &n) const {
    TGAColor normal_pixel = sample(mapping, uv);
    auto x = (normal_pixel[2] / 255.0 * 2) - 1;
    auto y = (normal_pixel[1] / 255.0 * 2) - 1;
    auto z = (normal_pixel[0] / 255.0 * 2) - 1;
    vec3 E_0 = tri[1] - tri[0];
    vec3 E_1 = tri[2] - tri[0];
    mat<3, 2> E;
    E.set_column(0, E_0);
    E.set_column(1, E_1);

    vec3 U_0 = tri_tex[1] - tri_tex[0];
    vec3 U_1 = tri_tex[2] - tri_tex[0];
    mat<2, 2> U;
    U.set_column(0, to_vec2(U_0));
    U.set_column(1, to_vec2(U_1));

    mat<3, 2> TB = E * inverse(U);
    vec3 t = unit_vector(TB.get_column(0));
    vec3 b = unit_vector(TB.get_column(1));

    vec3 normal = unit_vector(x * t + y * b + z * n);
    return normal;
  }

  virtual std::pair<bool, TGAColor> fragment(const vec3 barycentric) const {
    // Get normal values from _nm.tga
    vec3 uv = {barycentric.x * tri_tex[0] + barycentric.y * tri_tex[1] +
               barycentric.z * tri_tex[2]};
    vec3 n = unit_vector(barycentric.x * tri_normal[0] +
                         barycentric.y * tri_normal[1] +
                         barycentric.z * tri_normal[2]);
    vec3 normal =
        get_normal_from_nm_tangent_tga(model.get_normal_tangent_map(), uv, n);
    // vec3 normal = get_normal_from_nm_tga(model.get_normal_map(), uv);

    TGAColor diffuse_color = sample(model.get_diffuse_map(), uv);
    TGAColor specular_sample = sample(model.get_spec_map(), uv);
    double spec_strength = specular_sample[0] / 255.0;

    auto glow = sample(model.get_glow_map(), uv);

    // Calculate Phong reflection value
    auto diffuse = std::fmax(0.0, dot(normal, l));
    auto r = dot(normal, l) * 2 * normal - l;
    // v is always (0,0,1) because we build the camera on l,m,n and n is the
    // third basis vector
    auto e = 35;
    auto specular = std::pow(std::fmax(0.0, dot(r, vec3{0, 0, 1})), e);
    double ambient = 0.4;

    TGAColor result;
    for (int ch = 0; ch < 3; ch++) {
      double value = diffuse_color[ch] * (ambient + 0.6 * diffuse) +
                     3.0 * specular_color[ch] * spec_strength * specular +
                     glow[ch];
      result[ch] = static_cast<std::uint8_t>(std::fmin(255.0, value));
    }
    return {false, result};
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
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
  TGAImage framebuffer(width, height, TGAImage::RGB, {{177, 195, 255, 209}});
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
