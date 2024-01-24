// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/serializable.h"

namespace content {

Serializable::Serializable() {}

Serializable::~Serializable() {}

Serializable::ByteElement* Serializable::RawData(ByteType* data) {
  return &data->front();
}

int32_t Serializable::ReadInt32(ByteType* data, int offset) {
  int32_t result;
  memcpy(&result, RawData(data) + offset, sizeof(int32_t));
  return result;
}

double Serializable::ReadDouble(ByteType* data, int offset) {
  double result;
  memcpy(&result, RawData(data) + offset, sizeof(double));
  return result;
}

void Serializable::WriteInt32(ByteType* data, int offset, int32_t value) {
  memcpy(RawData(data) + offset, &value, sizeof(int32_t));
}

void Serializable::WriteDouble(ByteType* data, int offset, double value) {
  memcpy(RawData(data) + offset, &value, sizeof(double));
}

}  // namespace content
