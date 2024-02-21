// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_UTILITY_H_
#define CONTENT_PUBLIC_UTILITY_H_

#include <algorithm>

#include "base/bind/callback_list.h"
#include "base/math/math.h"
#include "base/memory/ref_counted.h"
#include "content/common/serializable.h"

namespace content {

class ValueNotification {
 public:
  virtual ~ValueNotification() = default;

  base::CallbackListSubscription AddChangedObserver(
      base::RepeatingClosure observer) {
    return observers_.Add(std::move(observer));
  }

 protected:
  inline void UpdateData() { observers_.Notify(); }

 private:
  base::RepeatingClosureList observers_;
};

class Rect : public base::RefCounted<Rect>,
             public Serializable,
             public ValueNotification {
 public:
  Rect() : x_(0), y_(0), width_(0), height_(0) {}
  Rect(const base::Rect& rect)
      : x_(rect.x), y_(rect.y), width_(rect.width), height_(rect.height) {}

  Rect(const Rect& other) {
    x_ = other.x_;
    y_ = other.y_;
    width_ = other.width_;
    height_ = other.height_;
  }

  const Rect& operator=(const Rect& other) {
    x_ = other.x_;
    y_ = other.y_;
    width_ = other.width_;
    height_ = other.height_;
    return other;
  }

  bool IsSame(const Rect& other) {
    return other.x_ == x_ && other.y_ == y_ && other.width_ == width_ &&
           other.height_ == height_;
  }

  void Set(const base::Rect& rect) {
    bool changed = false;
    if (x_ != rect.x)
      changed = true;
    x_ = rect.x;
    if (y_ != rect.y)
      changed = true;
    y_ = rect.y;
    if (width_ != rect.width)
      changed = true;
    width_ = rect.width;
    if (height_ != rect.height)
      changed = true;
    height_ = rect.height;
    if (changed)
      UpdateData();
  }

  void Set(scoped_refptr<Rect> rect) {
    bool changed = false;
    if (x_ != rect->x_)
      changed = true;
    x_ = rect->x_;
    if (y_ != rect->y_)
      changed = true;
    y_ = rect->y_;
    if (width_ != rect->width_)
      changed = true;
    width_ = rect->width_;
    if (height_ != rect->height_)
      changed = true;
    height_ = rect->height_;
    if (changed)
      UpdateData();
  }

  void Empty() { Set(base::Rect(0, 0, 0, 0)); }

  int GetX() const { return x_; }
  void SetX(int x) {
    if (x_ == x)
      return;
    x_ = x;
    UpdateData();
  }

  int GetY() const { return y_; }
  void SetY(int y) {
    if (y_ == y)
      return;
    y_ = y;
    UpdateData();
  }

  int GetWidth() const { return width_; }
  void SetWidth(int width) {
    if (width_ == width)
      return;
    width_ = width;
    UpdateData();
  }

  int GetHeight() const { return height_; }
  void SetHeight(int height) {
    if (height_ == height)
      return;
    height_ = height;
    UpdateData();
  }

  base::Rect AsBase() {
    if (width_ < 0) {
      width_ = -width_;
      x_ -= width_;
    }

    if (height_ < 0) {
      height_ = -height_;
      y_ -= height_;
    }

    return base::Rect(x_, y_, width_, height_);
  }

  bool IsValid() const { return width_ != 0 && height_ != 0; }

  std::string Serialize() override;
  static scoped_refptr<Rect> Deserialize(const std::string& data);

 private:
  int x_;
  int y_;
  int width_;
  int height_;
};

class Tone : public base::RefCounted<Tone>,
             public Serializable,
             public ValueNotification {
 public:
  Tone() : red_(0.0), green_(0.0), blue_(0.0), gray_(0.0) {}
  Tone(double red, double green, double blue, double gray = 255.0)
      : red_(std::clamp(red, -255.0, 255.0)),
        green_(std::clamp(green, -255.0, 255.0)),
        blue_(std::clamp(blue, -255.0, 255.0)),
        gray_(std::clamp(gray, 0.0, 255.0)) {}

  Tone(const Tone& other) {
    red_ = other.red_;
    green_ = other.green_;
    blue_ = other.blue_;
    gray_ = other.gray_;
  }

  const Tone& operator=(const Tone& other) {
    red_ = other.red_;
    green_ = other.green_;
    blue_ = other.blue_;
    gray_ = other.gray_;
    return other;
  }

  bool IsSame(const Tone& other) {
    return other.red_ == red_ && other.green_ == green_ &&
           other.blue_ == blue_ && other.gray_ == gray_;
  }

  void Set(double red, double green, double blue, double gray = 255.0f) {
    bool changed = false;
    double tmp;

    tmp = std::clamp(red, -255.0, 255.0);
    if (red_ != tmp)
      changed = true;
    red_ = tmp;
    tmp = std::clamp(green, -255.0, 255.0);
    if (green_ != tmp)
      changed = true;
    green_ = tmp;
    tmp = std::clamp(blue, -255.0, 255.0);
    if (blue_ != tmp)
      changed = true;
    blue_ = tmp;
    tmp = std::clamp(gray, -255.0, 255.0);
    if (gray_ != tmp)
      changed = true;
    gray_ = tmp;

    if (changed)
      UpdateData();
  }

  void Set(scoped_refptr<Tone> tone) {
    Set(tone->red_, tone->green_, tone->blue_, tone->gray_);
  }

  double GetRed() const { return red_; }
  void SetRed(double red) {
    double tmp = std::clamp(red, -255.0, 255.0);
    if (tmp == red_)
      return;
    red_ = tmp;
    UpdateData();
  }

  double GetGreen() const { return green_; }
  void SetGreen(double green) {
    double tmp = std::clamp(green, -255.0, 255.0);
    if (tmp == green_)
      return;
    green_ = tmp;
    UpdateData();
  }

  double GetBlue() const { return blue_; }
  void SetBlue(double blue) {
    double tmp = std::clamp(blue, -255.0, 255.0);
    if (tmp == blue_)
      return;
    blue_ = tmp;
    UpdateData();
  }

  double GetGray() const { return gray_; }
  void SetGray(double gray) {
    double tmp = std::clamp(gray, 0.0, 255.0);
    if (tmp == gray_)
      return;
    gray_ = tmp;
    UpdateData();
  }

  base::Vec4 AsBase() {
    return base::Vec4((float)red_ / 255.0f, (float)green_ / 255.0f,
                      (float)blue_ / 255.0f, (float)gray_ / 255.0f);
  }

  bool IsValid() {
    return red_ != 0.0 || green_ != 0.0 || blue_ != 0.0 || gray_ != 0.0;
  }

  std::string Serialize() override;
  static scoped_refptr<Tone> Deserialize(const std::string& data);

 private:
  double red_;
  double green_;
  double blue_;
  double gray_;
};

class Color : public base::RefCounted<Color>,
              public Serializable,
              public ValueNotification {
 public:
  Color() : red_(0.0), green_(0.0), blue_(0.0), alpha_(0.0) {}
  Color(double red, double green, double blue, double alpha = 255.0)
      : red_(std::clamp(red, 0.0, 255.0)),
        green_(std::clamp(green, 0.0, 255.0)),
        blue_(std::clamp(blue, 0.0, 255.0)),
        alpha_(std::clamp(alpha, 0.0, 255.0)) {}

  Color(const Color& other) {
    red_ = other.red_;
    green_ = other.green_;
    blue_ = other.blue_;
    alpha_ = other.alpha_;
  }

  const Color& operator=(const Color& other) {
    red_ = other.red_;
    green_ = other.green_;
    blue_ = other.blue_;
    alpha_ = other.alpha_;
    return other;
  }

  bool IsSame(const Color& other) {
    return other.red_ == red_ && other.green_ == green_ &&
           other.blue_ == blue_ && other.alpha_ == alpha_;
  }

  void Set(double red, double green, double blue, double alpha) {
    bool changed = false;
    double tmp;

    tmp = std::clamp(red, 0.0, 255.0);
    if (tmp != red_)
      changed = true;
    red_ = tmp;

    tmp = std::clamp(green, 0.0, 255.0);
    if (tmp != green_)
      changed = true;
    green_ = tmp;

    tmp = std::clamp(blue, 0.0, 255.0);
    if (tmp != blue_)
      changed = true;
    blue_ = tmp;

    tmp = std::clamp(alpha, 0.0, 255.0);
    if (tmp != alpha_)
      changed = true;
    alpha_ = tmp;

    if (changed)
      UpdateData();
  }

  void Set(scoped_refptr<Color> color) {
    Set(color->red_, color->green_, color->blue_, color->alpha_);
  }

  double GetRed() const { return red_; }
  void SetRed(double red) {
    double tmp = std::clamp(red, 0.0, 255.0);
    if (tmp == red_)
      return;
    red_ = tmp;
    UpdateData();
  }

  double GetGreen() const { return green_; }
  void SetGreen(double green) {
    double tmp = std::clamp(green, 0.0, 255.0);
    if (tmp == green_)
      return;
    green_ = tmp;
    UpdateData();
  }

  double GetBlue() const { return blue_; }
  void SetBlue(double blue) {
    double tmp = std::clamp(blue, 0.0, 255.0);
    if (tmp == blue_)
      return;
    blue_ = tmp;
    UpdateData();
  }

  double GetAlpha() const { return alpha_; }
  void SetAlpha(double alpha) {
    double tmp = std::clamp(alpha, 0.0, 255.0);
    if (tmp == alpha_)
      return;
    alpha_ = tmp;
    UpdateData();
  }

  base::Vec4 AsBase() {
    return base::Vec4((float)red_ / 255.0f, (float)green_ / 255.0f,
                      (float)blue_ / 255.0f, (float)alpha_ / 255.0f);
  }

  base::Vec4 AsNormal() {
    return base::Vec4(static_cast<float>(red_), static_cast<float>(green_),
                      static_cast<float>(blue_), static_cast<float>(alpha_));
  }
  SDL_Color AsSDLColor() {
    return SDL_Color{static_cast<uint8_t>(red_), static_cast<uint8_t>(green_),
                     static_cast<uint8_t>(blue_), static_cast<uint8_t>(alpha_)};
  }

  bool IsValid() { return alpha_ != 0.0; }

  std::string Serialize() override;
  static scoped_refptr<Color> Deserialize(const std::string& data);

 private:
  double red_;
  double green_;
  double blue_;
  double alpha_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_UTILITY_H_
