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
  Rect() {}
  Rect(const base::Rect& rect) : data_(rect) {}
  ~Rect() override {}

  Rect(const Rect& other) { data_ = other.data_; }
  Rect& operator=(const Rect& other) { data_ = other.data_; }
  bool operator==(const Rect& other) { return other.data_ == data_; }

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

  std::string Serialize() override;
  static scoped_refptr<Rect> Deserialize(const std::string& data);

 private:
  base::Rect data_;
};

class Tone : public base::RefCounted<Tone>,
             public Serializable,
             public ValueNotification {
 public:
  Tone() {}
  Tone(float red, float green, float blue, float gray = 255.0f)
      : data_(std::clamp(red, -255.0f, 255.0f),
              std::clamp(green, -255.0f, 255.0f),
              std::clamp(blue, -255.0f, 255.0f),
              std::clamp(gray, 0.0f, 255.0f)) {}
  ~Tone() override {}

  Tone(const Tone& other) { data_ = other.data_; }
  Tone& operator=(const Tone& other) { data_ = other.data_; }
  bool operator==(const Tone& other) { return other.data_ == data_; }

  void Set(float red, float green, float blue, float gray = 255.0f) {
    data_.x = std::clamp(red, -255.0f, 255.0f);
    data_.y = std::clamp(green, -255.0f, 255.0f);
    data_.z = std::clamp(blue, -255.0f, 255.0f);
    data_.w = std::clamp(gray, 0.0f, 255.0f);

    UpdateData();
  }

  void Set(scoped_refptr<Tone> tone) {
    data_ = tone->data_;
    UpdateData();
  }

  float GetRed() const { return data_.x; }
  void SetRed(float red) {
    data_.x = std::clamp(red, -255.0f, 255.0f);
    UpdateData();
  }

  float GetGreen() const { return data_.y; }
  void SetGreen(float green) {
    data_.y = std::clamp(green, -255.0f, 255.0f);
    UpdateData();
  }

  float GetBlue() const { return data_.z; }
  void SetBlue(float blue) {
    data_.z = std::clamp(blue, -255.0f, 255.0f);
    UpdateData();
  }

  float GetGray() const { return data_.w; }
  void SetGray(float gray) {
    data_.w = std::clamp(gray, 0.0f, 255.0f);
    UpdateData();
  }

  base::Vec4 AsBase() {
    return base::Vec4(data_.x / 255.0f, data_.y / 255.0f, data_.z / 255.0f,
                      data_.w / 255.0f);
  }

  base::Vec4& AsNormal() { return data_; }

  bool IsValid() {
    return data_.x != 0.0f || data_.y != 0.0f || data_.z != 0.0f ||
           data_.w != 0.0f;
  }

  std::string Serialize() override;
  static scoped_refptr<Tone> Deserialize(const std::string& data);

 private:
  base::Vec4 data_;
};

class Color : public base::RefCounted<Color>,
              public Serializable,
              public ValueNotification {
 public:
  Color() {}
  Color(float red, float green, float blue, float alpha = 255.0f)
      : data_(std::clamp(red, 0.0f, 255.0f),
              std::clamp(green, 0.0f, 255.0f),
              std::clamp(blue, 0.0f, 255.0f),
              std::clamp(alpha, 0.0f, 255.0f)) {}
  ~Color() override {}

  Color(const Color& other) { data_ = other.data_; }
  Color& operator=(const Color& other) { data_ = other.data_; }
  bool operator==(const Color& other) { return other.data_ == data_; }

  void Set(float red, float green, float blue, float alpha) {
    data_.x = std::clamp(red, 0.0f, 255.0f);
    data_.y = std::clamp(green, 0.0f, 255.0f);
    data_.z = std::clamp(blue, 0.0f, 255.0f);
    data_.w = std::clamp(alpha, 0.0f, 255.0f);

    UpdateData();
  }

  void Set(scoped_refptr<Color> color) {
    data_ = color->data_;
    UpdateData();
  }

  float GetRed() const { return data_.x; }
  void SetRed(float red) {
    data_.x = std::clamp(red, 0.0f, 255.0f);
    UpdateData();
  }

  float GetGreen() const { return data_.y; }
  void SetGreen(float green) {
    data_.y = std::clamp(green, 0.0f, 255.0f);
    UpdateData();
  }

  float GetBlue() const { return data_.z; }
  void SetBlue(float blue) {
    data_.z = std::clamp(blue, 0.0f, 255.0f);
    UpdateData();
  }

  float GetAlpha() const { return data_.w; }
  void SetAlpha(float alpha) {
    data_.w = std::clamp(alpha, 0.0f, 255.0f);
    UpdateData();
  }

  base::Vec4 AsBase() {
    return base::Vec4(data_.x / 255.0f, data_.y / 255.0f, data_.z / 255.0f,
                      data_.w / 255.0f);
  }

  base::Vec4& AsNormal() { return data_; }
  SDL_Color AsSDLColor() {
    return SDL_Color{
        static_cast<uint8_t>(data_.x), static_cast<uint8_t>(data_.y),
        static_cast<uint8_t>(data_.z), static_cast<uint8_t>(data_.w)};
  }

  bool IsValid() { return data_.w != 0.0f; }

  std::string Serialize() override;
  static scoped_refptr<Color> Deserialize(const std::string& data);

 private:
  base::Vec4 data_;
};

}  // namespace content

#endif  // !CONTENT_PUBLIC_UTILITY_H_
