#pragma once
#include <cassert>
#include <cmath>
#include <iostream>

template <int n> struct vec {
  double data[n] = {};
  double &operator[](const int i) {
    assert(i >= 0 && i < n);
    return data[i];
  }
  double operator[](const int i) const {
    assert(i >= 0 && i < n);
    return data[i];
  }
};

template <int n> std::ostream &operator<<(std::ostream &out, const vec<n> &v) {
  for (int i = 0; i < n; i++)
    out << v[i] << " ";
  return out;
}

template <int n> vec<n> operator+(const vec<n> &v1, const vec<n> &v2) {
  vec<n> result = v1;
  for (int i = 0; i < n; i++) {
    result[i] += v2[i];
  }
  return result;
}

template <int n> vec<n> operator-(const vec<n> &v1, const vec<n> &v2) {
  vec<n> result = v1;
  for (int i = 0; i < n; i++) {
    result[i] -= v2[i];
  }
  return result;
}

template <int n> double dot(const vec<n> &v1, const vec<n> &v2) {
  double result = 0.0;
  for (int i = 0; i < n; i++) {
    result += v1[i] * v2[i];
  }
  return result;
}

template <int n> vec<n> operator*(const vec<n> &v, const double scale) {
  vec<n> result = v;
  for (int i = 0; i < n; i++) {
    result[i] = v[i] * scale;
  }
  return result;
}

template <int n> vec<n> operator*(const double scale, const vec<n> &v) {
  return v * scale;
}

template <> struct vec<2> {
  double x = 0, y = 0;
  double &operator[](const int i) {
    assert(i >= 0 && i < 2);
    return i == 1 ? y : x;
  }
  double operator[](const int i) const {
    assert(i >= 0 && i < 2);
    return i == 1 ? y : x;
  }
};

template <> struct vec<3> {
  double x = 0, y = 0, z = 0;
  double &operator[](const int i) {
    assert(i >= 0 && i < 3);
    return i ? (1 == i ? y : z) : x;
  }
  double operator[](const int i) const {
    assert(i >= 0 && i < 3);
    return i ? (1 == i ? y : z) : x;
  }
};

template <> struct vec<4> {
  double x = 0, y = 0, z = 0, w = 0;
  double &operator[](const int i) {
    assert(i >= 0 && i < 4);
    switch (i) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      return w;
    }
  }
  double operator[](const int i) const {
    assert(i >= 0 && i < 4);
    switch (i) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      return w;
    }
  }
};

inline vec<3> cross(const vec<3> &v1, const vec<3> &v2) {
  return {(v1.y * v2.z) - (v2.y * v1.z), (v2.x * v1.z) - (v1.x * v2.z),
          (v1.x * v2.y) - (v2.x * v1.y)};
}

template <int n> double length_squared(const vec<n> &v) { return dot(v, v); }

template <int n> double length(const vec<n> &v) {
  return std::sqrt(length_squared(v));
}

template <int n> vec<n> unit_vector(const vec<n> &v) {
  vec<n> result = v;
  auto inverse_length = 1 / length(v);
  return result * inverse_length;
}

template <int n> vec<n> operator-(const vec<n> &v) { return -1 * v; }

template <int nrows, int ncols> struct mat {
  vec<ncols> rows[nrows] = {{}};
  vec<ncols> &operator[](const int idx) {
    assert(idx >= 0 && idx < nrows);
    return rows[idx];
  }

  const vec<ncols> &operator[](const int idx) const {
    assert(idx >= 0 && idx < nrows);
    return rows[idx];
  }
};

template <int r, int c> vec<c> operator*(const mat<r, c> &m, const vec<c> &v) {
  vec<r> result;
  for (int i = 0; i < r; i++) {
    result[i] = dot(m[i], v);
  }
  return result;
}

template <int r, int n, int c>
mat<r, c> operator*(const mat<r, n> &m1, const mat<n, c> &m2) {
  mat<r, c> result;
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      for (int k = 0; k < n; k++) {
        result[i][j] += m1[i][k] * m2[k][j];
      }
    }
  }
  return result;
}

template <int r, int c> mat<c, r> transpose(const mat<r, c> &m) {
  mat<c, r> result;
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      result[j][i] = m[i][j];
    }
  }
  return result;
}

template <int n>
mat<n - 1, n - 1> minor_matrix(const mat<n, n> &m, const int row,
                               const int col) {
  mat<n - 1, n - 1> result;
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - 1; j++) {
      result[i][j] = m[i < row ? i : i + 1][j < col ? j : j + 1];
    }
  }

  return result;
}

template <int n> double determinant(const mat<n, n> &m) {
  if constexpr (n == 1) {
    return m[0][0];
  } else {
    double result = 0.0;
    for (int j = 0; j < n; j++) {
      double sign = (j % 2 == 0) ? 1 : -1;
      result += sign * m[0][j] * determinant(minor_matrix(m, 0, j));
    }
    return result;
  }
}

template <int n> mat<n, n> adj(const mat<n, n> &m) {
  mat<n, n> result;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      double sign = ((j % 2 == 0) ? 1 : -1) * ((i % 2 == 0) ? 1 : -1);
      // Transpose in same step
      result[j][i] = sign * determinant(minor_matrix(m, i, j));
    }
  }
  return result;
}

template <int n> mat<n, n> inverse(const mat<n, n> &m) {
  auto det = determinant(m);
  assert(det != 0);
  return 1 / det * adj(m);
}

template <int r, int c> mat<r, c> operator*(const mat<r, c> &m, double scale) {
  mat<r, c> result;
  for (int i = 0; i < r; i++) {
    result[i] = m[i] * scale;
  }
  return result;
}

template <int r, int c> mat<r, c> operator*(double scale, const mat<r, c> &m) {
  return m * scale;
}

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;
