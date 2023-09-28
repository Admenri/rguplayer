#ifndef BASE_MATH_H_
#define BASE_MATH_H_

#include <SDL_rect.h>

namespace base {

class Rect;
class RectF;

class Vec2;
class Vec3;
class Vec4;

class Mat3;
class Mat4;

class Rect {
 public:
  Rect() : x(0), y(0), width(0), height(0) {}
  Rect(int ix, int iy, int iw, int ih) : x(ix), y(iy), width(iw), height(ih) {}
  virtual ~Rect() = default;

  Rect(const Rect&) = default;
  Rect& operator=(const Rect&) = default;

  SDL_Rect ToSDLRect();
  RectF ToFloatRect();

 public:
  int x, y, width, height;
};

class RectF {
 public:
  RectF() : x(0.f), y(0.f), width(0.f), height(0.f) {}
  RectF(float ix, float iy, float iw, float ih)
      : x(ix), y(iy), width(iw), height(ih) {}
  virtual ~RectF() = default;

  RectF(const RectF&) = default;
  RectF& operator=(const RectF&) = default;

 public:
  float x, y, width, height;
};

class Vec2 {
 public:
  Vec2() : x(0.f), y(0.f) {}
  Vec2(float ix, float iy) : x(ix), y(iy) {}
  virtual ~Vec2() = default;

  Vec2(const Vec2&) = default;
  Vec2& operator=(const Vec2&) = default;

  bool operator==(const Vec2& other) { return other.x == x && other.y == y; }
  Vec2 operator+(const Vec2& value) { return Vec2(x + value.x, y + value.y); }
  Vec2 operator-(const Vec2& value) { return Vec2(x - value.x, y - value.y); }

 public:
  float x, y;
};

class Vec2i {
 public:
  Vec2i() : x(0), y(0) {}
  Vec2i(int ix, int iy) : x(ix), y(iy) {}
  virtual ~Vec2i() = default;

  Vec2i(const Vec2i&) = default;
  Vec2i& operator=(const Vec2i&) = default;

  bool operator==(const Vec2i& other) { return other.x == x && other.y == y; }
  Vec2i operator+(const Vec2i& value) {
    return Vec2i(x + value.x, y + value.y);
  }
  Vec2i operator-(const Vec2i& value) {
    return Vec2i(x - value.x, y - value.y);
  }

 public:
  int x, y;
};

class Vec3 {
 public:
  Vec3() : x(0.f), y(0.f), z(0.f) {}
  Vec3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
  virtual ~Vec3() = default;

  Vec3(const Vec3&) = default;
  Vec3& operator=(const Vec3&) = default;

  bool operator==(const Vec3& other) {
    return other.x == x && other.y == y && other.z == z;
  }

  Vec3 operator+(const Vec3& value) {
    return Vec3(x + value.x, y + value.y, z + value.z);
  }

  Vec3 operator-(const Vec3& value) {
    return Vec3(x - value.x, y - value.y, z - value.z);
  }

 public:
  float x, y, z;
};

class Vec3i {
 public:
  Vec3i() : x(0.f), y(0.f), z(0.f) {}
  Vec3i(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {}
  virtual ~Vec3i() = default;

  Vec3i(const Vec3i&) = default;
  Vec3i& operator=(const Vec3i&) = default;

  bool operator==(const Vec3i& other) {
    return other.x == x && other.y == y && other.z == z;
  }

  Vec3i operator+(const Vec3i& value) {
    return Vec3i(x + value.x, y + value.y, z + value.z);
  }

  Vec3i operator-(const Vec3i& value) {
    return Vec3i(x - value.x, y - value.y, z - value.z);
  }

 public:
  int x, y, z;
};

class Vec4 {
 public:
  Vec4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
  Vec4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
  virtual ~Vec4() = default;

  Vec4(const Vec4&) = default;
  Vec4& operator=(const Vec4&) = default;

  bool operator==(const Vec4& other) {
    return other.x == x && other.y == y && other.z == z && other.w == w;
  }

  Vec4 operator+(const Vec4& value) {
    return Vec4(x + value.x, y + value.y, z + value.z, w + value.w);
  }

  Vec4 operator-(const Vec4& value) {
    return Vec4(x - value.x, y - value.y, z - value.z, w - value.w);
  }

 public:
  float x, y, z, w;
};

class Vec4i {
 public:
  Vec4i() : x(0.f), y(0.f), z(0.f), w(0.f) {}
  Vec4i(int ix, int iy, int iz, int iw) : x(ix), y(iy), z(iz), w(iw) {}
  virtual ~Vec4i() = default;

  Vec4i(const Vec4i&) = default;
  Vec4i& operator=(const Vec4i&) = default;

  bool operator==(const Vec4i& other) {
    return other.x == x && other.y == y && other.z == z && other.w == w;
  }

  Vec4i operator+(const Vec4i& value) {
    return Vec4i(x + value.x, y + value.y, z + value.z, w + value.w);
  }

  Vec4i operator-(const Vec4i& value) {
    return Vec4i(x - value.x, y - value.y, z - value.z, w - value.w);
  }

 public:
  int x, y, z, w;
};

class Mat3 {
 public:
  Mat3() {
    for (int i = 0; i < 9; i++) {
      data[i] = 0.f;
    }
  }
  Mat3(float d[9]) {
    for (int i = 0; i < 9; i++) {
      data[i] = d[i];
    }
  }
  virtual ~Mat3() = default;

  Mat3(const Mat3&) = default;
  Mat3& operator=(const Mat3&) = default;

  bool operator==(const Mat3& other) {
    for (int i = 0; i < 9; i++) {
      if (other.data[i] != data[i]) {
        return false;
      }
    }
    return true;
  }

  Mat3 operator+(const Mat3& value) {
    float result[9];
    for (int i = 0; i < 9; i++) {
      result[i] = data[i] + value.data[i];
    }
    return Mat3(result);
  }

  Mat3 operator-(const Mat3& value) {
    float result[9];
    for (int i = 0; i < 9; i++) {
      result[i] = data[i] - value.data[i];
    }
    return Mat3(result);
  }

  Mat3 operator*(const Mat3& value) {
    float result[9];
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        result[i * 3 + j] = 0.f;
        for (int k = 0; k < 3; k++) {
          result[i * 3 + j] += data[i * 3 + k] * value.data[k * 3 + j];
        }
      }
    }
    return Mat3(result);
  }

 public:
  float data[9];
};

class Mat4 {
 public:
  Mat4() {
    for (int i = 0; i < 16; i++) {
      data[i] = 0.f;
    }
  }
  Mat4(float d[16]) {
    for (int i = 0; i < 16; i++) {
      data[i] = d[i];
    }
  }
  virtual ~Mat4() = default;

  Mat4(const Mat4&) = default;
  Mat4& operator=(const Mat4&) = default;

  bool operator==(const Mat4& other) {
    for (int i = 0; i < 16; i++) {
      if (other.data[i] != data[i]) {
        return false;
      }
    }
    return true;
  }

  Mat4 operator+(const Mat4& value) {
    float result[16];
    for (int i = 0; i < 16; i++) {
      result[i] = data[i] + value.data[i];
    }
    return Mat4(result);
  }

  Mat4 operator-(const Mat4& value) {
    float result[16];
    for (int i = 0; i < 16; i++) {
      result[i] = data[i] - value.data[i];
    }
    return Mat4(result);
  }

  Mat4 operator*(const Mat4& value) {
    float result[16];
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        result[i * 4 + j] = 0.f;
        for (int k = 0; k < 4; k++) {
          result[i * 4 + j] += data[i * 4 + k] * value.data[k * 4 + j];
        }
      }
    }
    return Mat4(result);
  }

 public:
  float data[16];
};

}  // namespace base

#endif  // BASE_MATH_H_