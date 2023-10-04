// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_UTILITY_H_
#define MODULES_UTILITY_H_

#include <algorithm>

#include "base/math/math.h"

namespace modules {

class Rect {
 public:
  Rect();
  explicit Rect(int x, int y, int width, int height)
      : rect_(x, y, width, height) {}
  virtual ~Rect() = default;

  Rect(const Rect&) = default;
  Rect& operator=(const Rect&) = default;

  void Set(int x, int y, int width, int height) {
    rect_ = base::Rect(x, y, width, height);
  }
  void Set(Rect* other) { rect_ = other->rect_; }

  void Empty() { rect_ = base::Rect(); }

  int GetX() const { return rect_.x; }
  int GetY() const { return rect_.y; }
  int GetWidth() const { return rect_.width; }
  int GeiHeight() const { return rect_.height; }

  void SetX(int x) { rect_.x = x; }
  void SetY(int y) { rect_.y = y; }
  void SetWidth(int width) { rect_.width = width; }
  void SetHeight(int height) { rect_.height = height; }

  base::Rect AsBase() const { return rect_; }

 private:
  base::Rect rect_;
};

class Color {
 public:
  Color() {}
  explicit Color(int red, int green, int blue, int alpha = 255)
      : color_(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
               std::clamp(blue, 0, 255), std::clamp(alpha, 0, 255)) {}
  virtual ~Color() = default;

  Color(const Color&) = default;
  Color& operator=(const Color&) = default;

  void Set(int red, int green, int blue, int alpha) {
    color_ = base::Vec4i(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
                         std::clamp(blue, 0, 255), std::clamp(alpha, 0, 255));
  }
  void Set(Color* other) { color_ = other->color_; }

  void Empty() { color_ = base::Vec4i(); }

  int GetRed() const { return color_.x; }
  int GetGreen() const { return color_.y; }
  int GetBlue() const { return color_.z; }
  int GetAlpha() const { return color_.w; }

  void SetRed(int red) { color_.x = std::clamp(red, 0, 255); }
  void SetGreen(int green) { color_.y = std::clamp(green, 0, 255); }
  void SetBlue(int blue) { color_.z = std::clamp(blue, 0, 255); }
  void SetAlpha(int alpha) { color_.w = std::clamp(alpha, 0, 255); }

  base::Vec4 ToFloatColor() const {
    return base::Vec4(color_.x / 255.0f, color_.y / 255.0f, color_.z / 255.0f,
                      color_.w / 255.0f);
  }

 private:
  base::Vec4i color_;
};

}  // namespace modules

#endif  // MODULES_RECT_H_