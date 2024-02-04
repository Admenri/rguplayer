// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MATH_MATH_H_
#define BASE_MATH_MATH_H_

#include <SDL_rect.h>

#include "base/debug/logging.h"

namespace base {

class Vec2;
class Vec3;
class Vec4;

class Vec2i;
class Vec3i;
class Vec4i;

class Mat3;
class Mat4;

class Rect;
class RectF;

class Vec2i {
 public:
  Vec2i() : x(0), y(0) {}
  Vec2i(int ix, int iy) : x(ix), y(iy) {}

  Vec2i(const Vec2i& other) {
    x = other.x;
    y = other.y;
  }

  const Vec2i& operator=(const Vec2i& other) {
    x = other.x;
    y = other.y;
    return other;
  }

  bool operator==(const Vec2i& other) const {
    return other.x == x && other.y == y;
  }
  Vec2i operator+(const Vec2i& value) const {
    return Vec2i(x + value.x, y + value.y);
  }
  Vec2i operator-(const Vec2i& value) const {
    return Vec2i(x - value.x, y - value.y);
  }
  Vec2i operator*(const Vec2i& value) const {
    return Vec2i(x * value.x, y * value.y);
  }
  Vec2i operator/(const Vec2i& value) const {
    return Vec2i(x / value.x, y / value.y);
  }

  void inspect() const { LOG(INFO) << "Vec2i(" << x << ", " << y << ")"; }

 public:
  int x, y;
};

class Vec2 {
 public:
  Vec2() : x(0.f), y(0.f) {}
  Vec2(float ix, float iy) : x(ix), y(iy) {}
  Vec2(const Vec2i& iv)
      : x(static_cast<float>(iv.x)), y(static_cast<float>(iv.y)) {}

  Vec2(const Vec2& other) {
    x = other.x;
    y = other.y;
  }

  const Vec2& operator=(const Vec2& other) {
    x = other.x;
    y = other.y;
    return other;
  }

  bool operator==(const Vec2& other) const {
    return other.x == x && other.y == y;
  }
  Vec2 operator+(const Vec2& value) const {
    return Vec2(x + value.x, y + value.y);
  }
  Vec2 operator-(const Vec2& value) const {
    return Vec2(x - value.x, y - value.y);
  }
  Vec2 operator*(const Vec2& value) const {
    return Vec2(x * value.x, y * value.y);
  }
  Vec2 operator/(const Vec2& value) const {
    return Vec2(x / value.x, y / value.y);
  }

  void inspect() const { LOG(INFO) << "Vec2(" << x << ", " << y << ")"; }

 public:
  float x, y;
};

class Vec3i {
 public:
  Vec3i() : x(0), y(0), z(0) {}
  Vec3i(int ix, int iy, int iz) : x(ix), y(iy), z(iz) {}

  Vec3i(const Vec3i& other) {
    x = other.x;
    y = other.y;
    z = other.z;
  }

  const Vec3i& operator=(const Vec3i& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return other;
  }

  bool operator==(const Vec3i& other) const {
    return other.x == x && other.y == y && other.z == z;
  }

  Vec3i operator+(const Vec3i& value) const {
    return Vec3i(x + value.x, y + value.y, z + value.z);
  }

  Vec3i operator-(const Vec3i& value) const {
    return Vec3i(x - value.x, y - value.y, z - value.z);
  }

  Vec3i operator*(const Vec3i& value) const {
    return Vec3i(x * value.x, y * value.y, z * value.z);
  }

  Vec3i operator/(const Vec3i& value) const {
    return Vec3i(x / value.x, y / value.y, z / value.z);
  }

  void inspect() const {
    LOG(INFO) << "Vec3i(" << x << ", " << y << ", " << z << ")";
  }

 public:
  int x, y, z;
};

class Vec3 {
 public:
  Vec3() : x(0.f), y(0.f), z(0.f) {}
  Vec3(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
  Vec3(const Vec3i& iv)
      : x(static_cast<float>(iv.x)),
        y(static_cast<float>(iv.y)),
        z(static_cast<float>(iv.z)) {}

  Vec3(const Vec3& other) {
    x = other.x;
    y = other.y;
    z = other.z;
  }

  const Vec3& operator=(const Vec3& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return other;
  }

  bool operator==(const Vec3& other) const {
    return other.x == x && other.y == y && other.z == z;
  }

  Vec3 operator+(const Vec3& value) const {
    return Vec3(x + value.x, y + value.y, z + value.z);
  }

  Vec3 operator-(const Vec3& value) const {
    return Vec3(x - value.x, y - value.y, z - value.z);
  }

  Vec3 operator*(const Vec3& value) const {
    return Vec3(x * value.x, y * value.y, z * value.z);
  }

  Vec3 operator/(const Vec3& value) const {
    return Vec3(x / value.x, y / value.y, z / value.z);
  }

  void inspect() const {
    LOG(INFO) << "Vec3(" << x << ", " << y << ", " << z << ")";
  }

 public:
  float x, y, z;
};

class Vec4i {
 public:
  Vec4i() : x(0), y(0), z(0), w(0) {}
  Vec4i(int ix, int iy, int iz, int iw) : x(ix), y(iy), z(iz), w(iw) {}

  Vec4i(const Vec4i& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
  }

  const Vec4i& operator=(const Vec4i& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return other;
  }

  bool operator==(const Vec4i& other) const {
    return other.x == x && other.y == y && other.z == z && other.w == w;
  }

  Vec4i operator+(const Vec4i& value) const {
    return Vec4i(x + value.x, y + value.y, z + value.z, w + value.w);
  }

  Vec4i operator-(const Vec4i& value) const {
    return Vec4i(x - value.x, y - value.y, z - value.z, w - value.w);
  }

  Vec4i operator*(const Vec4i& value) const {
    return Vec4i(x * value.x, y * value.y, z * value.z, w * value.w);
  }

  Vec4i operator/(const Vec4i& value) const {
    return Vec4i(x / value.x, y / value.y, z / value.z, w / value.w);
  }

  void inspect() const {
    LOG(INFO) << "Vec4i(" << x << ", " << y << ", " << z << ", " << w << ")";
  }

 public:
  int x, y, z, w;
};

class Vec4 {
 public:
  Vec4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
  Vec4(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
  Vec4(const Vec4i& iv)
      : x(static_cast<float>(iv.x)),
        y(static_cast<float>(iv.y)),
        z(static_cast<float>(iv.z)),
        w(static_cast<float>(iv.w)) {}

  Vec4(const Vec4& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
  }

  const Vec4& operator=(const Vec4& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return other;
  }

  bool operator==(const Vec4& other) const {
    return other.x == x && other.y == y && other.z == z && other.w == w;
  }

  Vec4 operator+(const Vec4& value) const {
    return Vec4(x + value.x, y + value.y, z + value.z, w + value.w);
  }

  Vec4 operator-(const Vec4& value) const {
    return Vec4(x - value.x, y - value.y, z - value.z, w - value.w);
  }

  Vec4 operator*(const Vec4& value) const {
    return Vec4(x * value.x, y * value.y, z * value.z, w * value.w);
  }

  Vec4 operator/(const Vec4& value) const {
    return Vec4(x / value.x, y / value.y, z / value.z, w / value.w);
  }

  void inspect() const {
    LOG(INFO) << "Vec4(" << x << ", " << y << ", " << z << ", " << w << ")";
  }

 public:
  float x, y, z, w;
};

class Rect {
 public:
  Rect() : x(0), y(0), width(0), height(0) {}
  Rect(const Vec2i& size) : x(0), y(0), width(size.x), height(size.y) {}
  Rect(int ix, int iy, int iw, int ih) : x(ix), y(iy), width(iw), height(ih) {}
  Rect(const Vec2i& pos, const Vec2i& size)
      : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

  Rect(const Rect& other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
  }

  const Rect& operator=(const Rect& other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
    return other;
  };

  bool operator==(const Rect& other) const {
    return other.x == x && other.y == y && other.width == width &&
           other.height == height;
  }

  SDL_Rect ToSDLRect() const;
  RectF ToFloatRect() const;

  base::Vec2i Position() const { return base::Vec2i(x, y); }
  base::Vec2i Size() const { return base::Vec2i(width, height); }
  bool IsEnclosed(const Rect& other) const {
    return x <= other.x && y <= other.y &&
           (x + width >= other.x + other.width) &&
           (y + height >= other.y + other.height);
  }

  Rect operator+(const Rect& value) const {
    return Rect(x + value.x, y + value.y, width + value.width,
                height + value.height);
  }

  Rect operator-(const Rect& value) const {
    return Rect(x - value.x, y - value.y, width - value.width,
                height - value.height);
  }

  Rect operator*(const Rect& value) const {
    return Rect(x * value.x, y * value.y, width * value.width,
                height * value.height);
  }

  Rect operator/(const Rect& value) const {
    return Rect(x / value.x, y / value.y, width / value.width,
                height / value.height);
  }

  void inspect() const {
    LOG(INFO) << "Rect(" << x << ", " << y << ", " << width << ", " << height
              << ")";
  }

 public:
  int x, y, width, height;
};

class RectF {
 public:
  RectF() : x(0.f), y(0.f), width(0.f), height(0.f) {}
  RectF(const Vec2& size) : x(0), y(0), width(size.x), height(size.y) {}
  RectF(float ix, float iy, float iw, float ih)
      : x(ix), y(iy), width(iw), height(ih) {}
  RectF(const Vec2& pos, const Vec2& size)
      : x(pos.x), y(pos.y), width(size.x), height(size.y) {}
  RectF(const Rect& ir)
      : x(static_cast<float>(ir.x)),
        y(static_cast<float>(ir.y)),
        width(static_cast<float>(ir.width)),
        height(static_cast<float>(ir.height)) {}

  RectF(const RectF& other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
  }

  const RectF& operator=(const RectF& other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
    return other;
  }

  bool operator==(const RectF& other) const {
    return other.x == x && other.y == y && other.width == width &&
           other.height == height;
  }

  base::Vec2 Position() const { return base::Vec2(x, y); }
  base::Vec2 Size() const { return base::Vec2(width, height); }

  bool IsEnclosed(const RectF& other) const {
    return x <= other.x && y <= other.y &&
           (x + width >= other.x + other.width) &&
           (y + height >= other.y + other.height);
  }

  RectF operator+(const RectF& value) const {
    return RectF(x + value.x, y + value.y, width + value.width,
                 height + value.height);
  }

  RectF operator-(const RectF& value) const {
    return RectF(x - value.x, y - value.y, width - value.width,
                 height - value.height);
  }

  RectF operator*(const RectF& value) const {
    return RectF(x * value.x, y * value.y, width * value.width,
                 height * value.height);
  }

  RectF operator/(const RectF& value) const {
    return RectF(x / value.x, y / value.y, width / value.width,
                 height / value.height);
  }

  void inspect() const {
    LOG(INFO) << "RectF(" << x << ", " << y << ", " << width << ", " << height
              << ")";
  }

 public:
  float x, y, width, height;
};

}  // namespace base

#endif  // BASE_MATH_H_