// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SCRIPT_UTILITY_H_
#define CONTENT_SCRIPT_UTILITY_H_

#include <algorithm>

#include "base/bind/callback_list.h"
#include "base/math/math.h"
#include "base/memory/ref_counted.h"

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

class Rect : public base::RefCountedThreadSafe<Rect>, public ValueNotification {
 public:
  Rect() {}
  Rect(const base::Rect& rect) : data_(rect) {}
  ~Rect() override {}

  Rect(const Rect&) = default;
  Rect& operator=(const Rect&) = default;

  void Set(const base::Rect& rect) {
    data_ = rect;
    UpdateData();
  }

  void Set(scoped_refptr<Rect> rect) { Set(rect->data_); }
  void Empty() { Set(base::Rect()); }

  int GetX() const { return data_.x; }
  void SetX(int x) {
    data_.x = x;
    UpdateData();
  }

  int GetY() const { return data_.y; }
  void SetY(int y) {
    data_.y = y;
    UpdateData();
  }

  int GetWidth() const { return data_.width; }
  void SetWidth(int width) {
    data_.width = width;
    UpdateData();
  }

  int GetHeight() const { return data_.height; }
  void SetHeight(int height) {
    data_.height = height;
    UpdateData();
  }

  base::Rect& AsBase() {
    if (data_.width < 0) {
      data_.width = -data_.width;
      data_.x -= data_.width;
    }

    if (data_.height < 0) {
      data_.height = -data_.height;
      data_.y -= data_.height;
    }

    return data_;
  }

  void SetBase(const base::Rect& rect) { data_ = rect; }

 private:
  base::Rect data_;
};

class Tone : public base::RefCountedThreadSafe<Rect>, public ValueNotification {
 public:
  Tone() {}
  Tone(int red, int green, int blue, int gray = 255)
      : data_(std::clamp(red, 0, 255) / 255.0f,
              std::clamp(green, 0, 255) / 255.0f,
              std::clamp(blue, 0, 255) / 255.0f,
              std::clamp(gray, 0, 255) / 255.0f) {}
  ~Tone() override {}

  Tone(const Tone&) = default;
  Tone& operator=(const Tone&) = default;

  void Set(int red, int green, int blue, int gray) {
    data_.x = std::clamp(red, 0, 255) / 255.0f;
    data_.y = std::clamp(green, 0, 255) / 255.0f;
    data_.z = std::clamp(blue, 0, 255) / 255.0f;
    data_.w = std::clamp(gray, 0, 255) / 255.0f;

    UpdateData();
  }

  void Set(scoped_refptr<Tone> tone) {
    data_ = tone->data_;
    UpdateData();
  }

  int GetRed() const { return data_.x * 255; }
  void SetRed(int red) {
    data_.x = std::clamp(red, 0, 255) / 255.0f;
    UpdateData();
  }

  int GetGreen() const { return data_.y * 255; }
  void SetGreen(int green) {
    data_.y = std::clamp(green, 0, 255) / 255.0f;
    UpdateData();
  }

  int GetBlue() const { return data_.z * 255; }
  void SetBlue(int blue) {
    data_.z = std::clamp(blue, 0, 255) / 255.0f;
    UpdateData();
  }

  int GetGray() const { return data_.w * 255; }
  void SetGray(int gray) {
    data_.w = std::clamp(gray, 0, 255) / 255.0f;
    UpdateData();
  }

  base::Vec4& AsBase() { return data_; }

 private:
  base::Vec4 data_;
};

class Color : public base::RefCountedThreadSafe<Rect>,
              public ValueNotification {
 public:
  Color() {}
  Color(int red, int green, int blue, int alpha = 255)
      : data_(std::clamp(red, 0, 255), std::clamp(green, 0, 255),
              std::clamp(blue, 0, 255), std::clamp(alpha, 0, 255)) {}
  ~Color() override {}

  Color(const Color&) = default;
  Color& operator=(const Color&) = default;

  void Set(int red, int green, int blue, int alpha) {
    data_.x = std::clamp(red, 0, 255);
    data_.y = std::clamp(green, 0, 255);
    data_.z = std::clamp(blue, 0, 255);
    data_.w = std::clamp(alpha, 0, 255);

    UpdateData();
  }

  void Set(scoped_refptr<Color> color) {
    data_ = color->data_;
    UpdateData();
  }

  int GetRed() const { return data_.x; }
  void SetRed(int red) {
    data_.x = std::clamp(red, 0, 255);
    UpdateData();
  }

  int GetGreen() const { return data_.y; }
  void SetGreen(int green) {
    data_.y = std::clamp(green, 0, 255);
    UpdateData();
  }

  int GetBlue() const { return data_.z * 255; }
  void SetBlue(int blue) {
    data_.z = std::clamp(blue, 0, 255);
    UpdateData();
  }

  int GetAlpha() const { return data_.w * 255; }
  void SetAlpha(int alpha) {
    data_.w = std::clamp(alpha, 0, 255);
    UpdateData();
  }

  base::Vec4& AsBase() {
    value_ = base::Vec4(data_.x / 255.0f, data_.y / 255.0f, data_.z / 255.0f,
                        data_.w / 255.0f);
    return value_;
  }

  base::Vec4i& AsNormal() { return data_; }

 private:
  base::Vec4 value_;
  base::Vec4i data_;
};

}  // namespace content

#endif  // CONTENT_SCRIPT_UTILITY_H_