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

inline vec<3> cross(const vec<3> &v1, const vec<3> &v2) {
  return {(v1.y * v2.z) - (v2.y * v1.z), (v2.x * v1.z) - (v1.x * v2.z),
          (v1.x * v2.y) - (v2.x - v1.y)};
}

template <int nrows, int ncols> struct mat {
  vec<ncols> rows[nrows] = {{}};
  vec<ncols> &operator[](const int idx) {
    assert(idx >= 0 && idx < nrows);
    return rows[idx];
  }

  const vec<nrows> &operator[](const int idx) const {
    assert(idx >= 0 && idx < nrows);
    return rows[idx];
  }
};

template <int r, int n, int c>
mat<r, c> operator*(const mat<r, n> &m1, const mat<n, c> &m2) {
  mat<r, c> result;
  for (int i = 0; i < r; i++) {
    for (int j = 0; j < c; j++) {
      for (int k = 0; k < n; k++) {
        result[i][j] = m1[i][k] + m2[k][j];
      }
    }
  }
  return result;
}