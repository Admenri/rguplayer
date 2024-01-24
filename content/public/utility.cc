// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "content/public/utility.h"

namespace content {

std::unique_ptr<Serializable::ByteType> Rect::Serialize() {
  std::unique_ptr<ByteType> data = std::make_unique<ByteType>();

  data->resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.get(), 0, data_.x);
  Serializable::WriteInt32(data.get(), 4, data_.y);
  Serializable::WriteInt32(data.get(), 8, data_.width);
  Serializable::WriteInt32(data.get(), 12, data_.height);

  return data;
}

scoped_refptr<Rect> Rect::Deserialize(std::unique_ptr<ByteType> data) {
  int x, y, w, h;
  x = Serializable::ReadInt32(data.get(), 0);
  y = Serializable::ReadInt32(data.get(), 4);
  w = Serializable::ReadInt32(data.get(), 8);
  h = Serializable::ReadInt32(data.get(), 12);

  return new Rect(base::Rect(x, y, w, h));
}

std::unique_ptr<Serializable::ByteType> Tone::Serialize() {
  std::unique_ptr<ByteType> data = std::make_unique<ByteType>();

  data->resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.get(), 0, data_.x);
  Serializable::WriteInt32(data.get(), 4, data_.y);
  Serializable::WriteInt32(data.get(), 8, data_.z);
  Serializable::WriteInt32(data.get(), 12, data_.w);

  return data;
}

scoped_refptr<Tone> Tone::Deserialize(std::unique_ptr<ByteType> data) {
  int x, y, z, w;
  x = Serializable::ReadInt32(data.get(), 0);
  y = Serializable::ReadInt32(data.get(), 4);
  z = Serializable::ReadInt32(data.get(), 8);
  w = Serializable::ReadInt32(data.get(), 12);

  return new Tone(x, y, z, w);
}

std::unique_ptr<Serializable::ByteType> Color::Serialize() {
  std::unique_ptr<ByteType> data = std::make_unique<ByteType>();

  data->resize(4 * sizeof(int32_t));
  Serializable::WriteInt32(data.get(), 0, data_.x);
  Serializable::WriteInt32(data.get(), 4, data_.y);
  Serializable::WriteInt32(data.get(), 8, data_.z);
  Serializable::WriteInt32(data.get(), 12, data_.w);

  return data;
}

scoped_refptr<Color> Color::Deserialize(std::unique_ptr<ByteType> data) {
  int x, y, z, w;
  x = Serializable::ReadInt32(data.get(), 0);
  y = Serializable::ReadInt32(data.get(), 4);
  z = Serializable::ReadInt32(data.get(), 8);
  w = Serializable::ReadInt32(data.get(), 12);

  return new Color(x, y, z, w);
}

}  // namespace content
