// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULES_UTILITY_H_
#define MODULES_UTILITY_H_

#include <algorithm>

#include "base/bind/callback_list.h"
#include "base/math/math.h"
#include "base/memory/ref_counted.h"

namespace modules {

class Rect : public base::RefCounted<Rect> {
 public:
  Rect() {}
  Rect(const base::Rect& rect) : rect_(rect) {}
  explicit Rect(int x, int y, int width, int height)
      : rect_(x, y, width, height) {}
  virtual ~Rect() = default;

  void Set(int x, int y, int width, int height) {
    rect_ = base::Rect(x, y, width, height);
    value_observer_.Notify();
  }
  void Set(Rect* other) {
    rect_ = other->rect_;
    value_observer_.Notify();
  }

  void Empty() {
    rect_ = base::Rect();
    value_observer_.Notify();
  }

  int GetX() const { return rect_.x; }
  int GetY() const { return rect_.y; }
  int GetWidth() const { return rect_.width; }
  int GetHeight() const { return rect_.height; }

  void SetX(int x) {
    rect_.x = x;
    value_observer_.Notify();
  }
  void SetY(int y) {
    rect_.y = y;
    value_observer_.Notify();
  }
  void SetWidth(int width) {
    rect_.width = width;
    value_observer_.Notify();
  }
  void SetHeight(int height) {
    rect_.height = height;
    value_observer_.Notify();
  }

  base::Rect AsBase() const { return rect_; }

  base::CallbackListSubscription AddObserver(
      base::RepeatingCallback<void()> callback) {
    return value_observer_.Add(callback);
  }

  void SetBase(const base::Rect& rect) { rect_ = rect; }

 private:
  base::RepeatingCallbackList<void()> value_observer_;

  base::Rect rect_;
};

class Color : public base::RefCounted<Color> {
 public:
  Color() {}
  explicit Color(int red, int green, int blue, int alpha = 255)
      : color_(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
               std::clamp(blue, 0, 255), std::clamp(alpha, 0, 255)) {}
  virtual ~Color() = default;

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

  base::Vec4i ToBase() { return color_; }

 private:
  base::Vec4i color_;
};

class Tone : public base::RefCounted<Tone> {
 public:
  Tone() {}
  explicit Tone(int red, int green, int blue, int gray = 0)
      : tone_(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
              std::clamp(blue, 0, 255), std::clamp(gray, 0, 255)) {}
  virtual ~Tone() = default;

  void Set(int red, int green, int blue, int gray) {
    tone_ = base::Vec4i(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
                        std::clamp(blue, 0, 255), std::clamp(gray, 0, 255));
  }
  void Set(Tone* other) { tone_ = other->tone_; }

  void Empty() { tone_ = base::Vec4i(); }

  int GetRed() const { return tone_.x; }
  int GetGreen() const { return tone_.y; }
  int GetBlue() const { return tone_.z; }
  int GetGray() const { return tone_.w; }

  void SetRed(int red) { tone_.x = std::clamp(red, 0, 255); }
  void SetGreen(int green) { tone_.y = std::clamp(green, 0, 255); }
  void SetBlue(int blue) { tone_.z = std::clamp(blue, 0, 255); }
  void SetGray(int gray) { tone_.w = std::clamp(gray, 0, 255); }

  base::Vec4 ToFloatTone() const {
    return base::Vec4(tone_.x / 255.0f, tone_.y / 255.0f, tone_.z / 255.0f,
                      tone_.w / 255.0f);
  }

 private:
  base::Vec4i tone_;
};

}  // namespace modules

#endif  // MODULES_RECT_H_