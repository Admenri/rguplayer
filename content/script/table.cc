// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/script/table.h"

namespace content {

Table::Table(int x_size, int y_size, int z_size)
    : x_size_(x_size),
      y_size_(y_size),
      z_size_(z_size),
      data_(x_size * y_size * z_size) {}

Table::~Table() {}

Table::Table(const Table& other)
    : x_size_(other.x_size_),
      y_size_(other.y_size_),
      z_size_(other.z_size_),
      data_(other.data_) {}

Table& Table::operator=(const Table& other) {
  x_size_ = other.x_size_;
  y_size_ = other.y_size_;
  z_size_ = other.z_size_;

  data_ = other.data_;

  return *this;
}

int16_t Table::Get(int x, int y, int z) const {
  return data_[x + y * x_size_ + z * y_size_ * x_size_];
}

void Table::Set(int16_t value, int x, int y, int z) {
  if (x < 0 || x >= x_size_ || y < 0 || y >= y_size_ || z < 0 || z >= z_size_)
    return;

  data_[x + y * x_size_ + z * y_size_ * x_size_] = value;
}

void Table::Resize(int x) { Resize(x, y_size_); }

void Table::Resize(int x, int y) { Resize(x, y, z_size_); }

void Table::Resize(int x, int y, int z) {
  std::vector<int16_t> new_data(x * y * z);

  for (int k = 0; k < std::min(z, z_size_); ++k) {
    for (int j = 0; j < std::min(y, y_size_); ++j) {
      for (int i = 0; i < std::min(x, x_size_); ++i) {
        new_data[i + j * x + k * y * x] = At(i, j, k);
      }
    }
  }

  data_.swap(new_data);

  x_size_ = x;
  y_size_ = y;
  z_size_ = z;
}

}  // namespace content
