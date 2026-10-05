#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"
#include "tgaimage.h"

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

struct Corner {
  int v;  // Vertex
  int vt; // Texture
  int vn; // Normal
};

struct Face {
  Corner corners[3];
};

class Model {
  std::vector<vec3> vertices;
  std::vector<vec3> textures;
  std::vector<vec3> normals;
  std::vector<Face> faces;
  TGAImage normal_map;

public:
  Model(const std::string filename) : normal_map(load_normal_map(filename)) {
    load_model(filename);

    // Check if all files loaded correctly
    for (const Face &face : faces) {
      for (const Corner &c : face.corners) {
        assert(c.v >= 0 && c.v < (int)vertices.size());
        assert(c.vt >= 0 && c.vt < (int)textures.size());
        assert(c.vn >= 0 && c.vn < (int)normals.size());

        if (c.v < 0 || c.v >= (int)vertices.size() || c.vt < 0 ||
            c.vt >= (int)textures.size() || c.vn < 0 ||
            c.vn >= (int)normals.size()) {
          std::cerr << "Bad index in " << filename << std::endl;
          std::abort();
        }
      }
    }
    normal_map = get_normal_map();
  }

  // number of vertices
  int num_vertices() const { return vertices.size(); }

  // number of faces
  int num_faces() const { return faces.size(); }

  // 0 <= i < nverts()
  vec3 vert(const int i) const { return vertices[i]; }

  // 0 <= iface <= nfaces(), 0 <= nthvert < 3
  vec3 vert(const int iface, const int nthvert) const {
    return vertices[faces[iface].corners[nthvert].v];
  }

  vec3 tex(const int iface, const int nthvert) const {
    return textures[faces[iface].corners[nthvert].vt];
  }

  vec3 normal(const int iface, const int nthvert) const {
    return normals[faces[iface].corners[nthvert].vn];
  }

  TGAImage get_normal_map() const { return normal_map; }

private:
  void load_model(std::string filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
      std::cerr << "Error opening file" << std::endl;
      return;
    }

    char slash;

    std::string row;
    while (std::getline(file, row)) {
      std::istringstream iss(row);
      std::string type;
      iss >> type;

      if (type == "v") {
        vec3 vertex;
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

      } else if (type == "vn") {
        vec3 normal;
        iss >> normal.x >> normal.y >> normal.z;
        normals.push_back(normal);
      } else if (type == "vt") {
        vec3 tex;
        iss >> tex.x >> tex.y >> tex.z;
        textures.push_back(tex);
      }
    }
  }

  static TGAImage load_normal_map(const std::string &filename) {
    std::filesystem::path path(filename);
    path.replace_extension(); // drops ".obj"
    path += "_nm.tga";

    TGAImage image;
    if (!image.read_tga_file(path.string())) {
      std::cerr << "Error opening " << path << "\n";
    }
    image.flip_vertically();

    return image;
  }
};

#endif
