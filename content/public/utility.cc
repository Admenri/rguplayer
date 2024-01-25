// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/utility.h"

namespace content {

std::string Rect::Serialize() {
  std::string data;

  data.resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.data(), 0, data_.x);
  Serializable::WriteInt32(data.data(), 4, data_.y);
  Serializable::WriteInt32(data.data(), 8, data_.width);
  Serializable::WriteInt32(data.data(), 12, data_.height);

  return data;
}

scoped_refptr<Rect> Rect::Deserialize(const std::string& data) {
  int x, y, w, h;
  x = Serializable::ReadInt32(data.data(), 0);
  y = Serializable::ReadInt32(data.data(), 4);
  w = Serializable::ReadInt32(data.data(), 8);
  h = Serializable::ReadInt32(data.data(), 12);

  return new Rect(base::Rect(x, y, w, h));
}

std::string Tone::Serialize() {
  std::string data;

  data.resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.data(), 0, data_.x);
  Serializable::WriteInt32(data.data(), 4, data_.y);
  Serializable::WriteInt32(data.data(), 8, data_.z);
  Serializable::WriteInt32(data.data(), 12, data_.w);

  return data;
}

scoped_refptr<Tone> Tone::Deserialize(const std::string& data) {
  int x, y, z, w;
  x = Serializable::ReadInt32(data.data(), 0);
  y = Serializable::ReadInt32(data.data(), 4);
  z = Serializable::ReadInt32(data.data(), 8);
  w = Serializable::ReadInt32(data.data(), 12);

  return new Tone(x, y, z, w);
}

std::string Color::Serialize() {
  std::string data;

  data.resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.data(), 0, data_.x);
  Serializable::WriteInt32(data.data(), 4, data_.y);
  Serializable::WriteInt32(data.data(), 8, data_.z);
  Serializable::WriteInt32(data.data(), 12, data_.w);

  return data;
}

scoped_refptr<Color> Color::Deserialize(const std::string& data) {
  int x, y, z, w;
  x = Serializable::ReadInt32(data.data(), 0);
  y = Serializable::ReadInt32(data.data(), 4);
  z = Serializable::ReadInt32(data.data(), 8);
  w = Serializable::ReadInt32(data.data(), 12);

  return new Color(x, y, z, w);
}

}  // namespace content
