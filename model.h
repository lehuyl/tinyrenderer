#ifndef MODEL_H
#define MODEL_H

#include "geometry.h"

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
  std::vector<vec3> normals;
  std::vector<Face> faces;

public:
  Model(const std::string filename) { load_model(filename); }

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

  vec3 normal(const int iface, const int nthvert) const {
    return normals[faces[iface].corners[nthvert].vn];
  }

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
      }
    }
  }
};

#endif
